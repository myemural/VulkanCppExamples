#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D uSourceImage; // rgb = color, a = signed CoC (half-res pixels)

layout(push_constant) uniform BloomPassPushConstants {
    vec2 srcTexelSize;    // 1.0 / source image size
    vec2 dstTexelSize;    // 1.0 / target mip size
    float focusDistance;  // Focus distance in meters (view-space units)
    float cocScale;       // CoC radius (in half-res pixels) of an object at infinity
    float maxCocRadius;   // Maximum CoC radius in half-res pixels
    float bokehThreshold; // Not used in this pass
} pc;

const float PI = 3.14159265359;

// Kernel: 1 center sample + rings with "kRingDensity * ringıdex" samples (1 + 7 + 14 + 21 + 28 = 71 samples)
const int kRingCount = 4;
const int kRingDensity = 7;

// Foreground search: 3 rings with 8 samples
const int kSearchRingCount = 3;
const int kSearchRingSamples = 8;

vec2 ringOffset(int ring, int index, int sampleCount, float ringRadius)
{
    // Odd rings are rotated a little for a better spread
    float angle = 2.0 * PI * (float(index) + ((ring % 2 == 0) ? 0.0 : 0.5)) / float(sampleCount);
    return ringRadius * vec2(cos(angle), sin(angle));
}

// Number of kernel samples inside a disk with th given radius (continuous approximation of the ring layout)
float kernelSampleCount(float radius, float kernelRadius)
{
    float ring = radius * float(kRingCount) / kernelRadius;
    return 1.0 + 0.5 * float(kRingDensity) * ring * (ring + 1.0);
}

// Own layer: Blurs the center's own depth layer, kernel radius is the center's CoC
vec3 gatherOwnLayer(vec2 uv, float centerCoc)
{
    float layerSign = sign(centerCoc); // 1: Far field, -1: Near field
    float kernelRadius = abs(centerCoc);
    float edgeMargin = max(kernelRadius / float(kRingCount), 1.0);

    vec4 accum = vec4(0.0);
    for (int ring = 0; ring <= kRingCount; ++ring)
    {
        int sampleCount = (ring == 0) ? 1 : kRingDensity * ring;
        float radius = kernelRadius * float(ring) / float(kRingCount);

        for (int i = 0; i < sampleCount; ++i)
        {
            vec4 sampleValue = texture(uSourceImage, uv + ringOffset(ring, i, sampleCount, radius) * pc.srcTexelSize);
            float layerCoc = max(min(kernelRadius, layerSign * sampleValue.a), 0.0);
            float weight = clamp((layerCoc - radius + edgeMargin) / edgeMargin, 0.0, 1.0);
            accum += vec4(sampleValue.rgb, 1.0) * weight;
        }
    }

    // Center sample always has a weight > 0, so  "accum.a" cannot be zero
    return accum.rgb / accum.a;
}

// Finds the largest near filed CoC around the center
float searchNearCoc(vec2 uv)
{
    float maxNearCoc = 0.0;
    for (int ring = 1; ring <= kSearchRingCount; ++ring)
    {
        float radius = pc.maxCocRadius * float(ring) / float(kSearchRingCount);
        for (int i = 0; i < kSearchRingSamples; ++i)
        {
            vec2 offset = ringOffset(ring, i, kSearchRingSamples, radius);
            maxNearCoc = max(maxNearCoc, -texture(uSourceImage, uv + offset * pc.srcTexelSize).a);
        }
    }
    return maxNearCoc;
}

// Foreground layer: Near field samples in front of the center spread over the center if their CoC reaches it
vec4 gatherForegroundLayer(vec2 uv, float kernelRadius, float centerNearCoc)
{
    float edgeMargin = max(kernelRadius / float(kRingCount), 1.0);

    vec4 accum = vec4(0.0);
    for (int ring = 0; ring <= kRingCount; ++ring)
    {
        int sampleCount = (ring == 0) ? 1 : kRingDensity * ring;
        float radius = kernelRadius * float(ring) / float(kRingCount);

        for (int i = 0; i < sampleCount; ++i)
        {
            vec4 sampleValue = texture(uSourceImage, uv + ringOffset(ring, i, sampleCount, radius) * pc.srcTexelSize);
            float nearCoc = max(-sampleValue.a, 0.0);
            float weight = clamp((nearCoc - radius + edgeMargin) / edgeMargin, 0.0, 1.0);

            // Only the samples in front of the center (in-focus pixels have zero near CoC)
            weight *= smoothstep(centerNearCoc + 0.5, centerNearCoc + 1.5, nearCoc);

            // Each sample distributes its color over its own CoC disk, so its contribution is divided by the number of
            // the kernel samples inside that disk
            weight /= kernelSampleCount(nearCoc, kernelRadius);
            accum += vec4(sampleValue.rgb, 1.0) * weight;
        }
    }

    // "accum.a" is the foreground coverage of the center
    return accum.a > 0.0 ? vec4(accum.rgb / accum.a, clamp(accum.a, 0.0, 1.0)) : vec4(0.0);
}

void main()
{
    vec2 uv = gl_FragCoord.xy * pc.dstTexelSize;
    vec4 center = texture(uSourceImage, uv);

    // Own layer (only for the blurred pixels, in-focus pixels are taken from the full-res image in the composite pass)
    float ownAlpha = smoothstep(1.0, 2.0, abs(center.a));
    vec3 ownColor = ownAlpha > 0.0 ? gatherOwnLayer(uv, center.a) : center.rgb;

    // Foreground layer (only if there is a larger near field CoC around)
    float centerNearCoc = max(-center.a, 0.0);
    float foregroundKernelRadius = searchNearCoc(uv);
    vec4 foreground = vec4(0.0);
    if (foregroundKernelRadius > centerNearCoc + 0.5) {
        foreground = gatherForegroundLayer(uv, foregroundKernelRadius, centerNearCoc);
    }

    vec3 foregroundColor = foreground.a > 0.0 ? foreground.rgb : ownColor;

    // Own color on in-focus pixels is not valid, so the foreground color is used for them
    // Otherwise, the foreground would be mixed with a dark color at its edges
    ownColor = mix(foregroundColor, ownColor, ownAlpha);

    // Foreground layer over the own layer
    outColor = vec4(mix(ownColor, foregroundColor, foreground.a), foreground.a);
}
