#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D gLightPassOutput;
layout(set = 0, binding = 1) uniform sampler2D gRoughnessMetallic; // b: Emissive flag

layout(push_constant) uniform BrightPassPushConstants {
    vec2 invOutputSize;      // 1.0 / (half-res bloom target size)
    float exposure;          // Manual exposure in EV
    float threshold;         // Brightness threshold (in exposed units)
    float softKnee;          // Soft knee amount in range [0, 1]
    uint source;             // 0: Luminance threshold, 1: Emissive Mask
} pc;

const uint SOURCE_THRESHOLD = 0U;
const uint SOURCE_EMISSIVE = 1U;

// Upper limit for the brightness of a bloom sample
const float kMaxBrightness = 64.0;

float maxComponent(vec3 color)
{
    return max(color.r, max(color.g, color.b));
}

// Soft-knee threshold: Instead of hard cut-off at the threshold, brightness values in
// "[threshold - knee, threshold + knee]" fade in with a quadratic curve
vec3 applySoftThreshold(vec3 color, float threshold, float softKnee)
{
    float brightness = maxComponent(color);
    float knee = threshold * softKnee + 1e-5;

    float soft = clamp(brightness - threshold + knee, 0.0, 2.0 * knee);
    soft = (soft * soft) / (4.0 * knee);

    float contribution = max(soft, brightness - threshold) / max(brightness, 1e-5);
    return color * contribution;
}

void main()
{
    vec2 uv = gl_FragCoord.xy * pc.invOutputSize;

    // Every bilinear tap averages a 2x2 full-res block, so for taps give a 4x4 box filter
    // It is more stable than a single tap while moving the camera
    vec2 fullResTexel = 1.0 / vec2(textureSize(gLightPassOutput, 0));
    vec2 offsets[4] = vec2[](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));

    vec3 color = vec3(0.0);
    float emissiveMask = 0.0;
    for (int i = 0; i < 4; ++i) {
        vec2 sampleUv = uv + offsets[i] * fullResTexel;
        color += texture(gLightPassOutput, sampleUv).rgb;
        emissiveMask += texture(gRoughnessMetallic, sampleUv).b;
    }
    color *= 0.25;
    emissiveMask *= 0.25;

    // Bloom is extracted from the exposed color, so the threshold stays meaningful while the exposure changes
    color *= exp2(pc.exposure);

    vec3 bloomColor;
    if (pc.source == SOURCE_EMISSIVE) {
        // Selective glow: Only emissive surfaces contribute, the mask gives a smooth coverage on their edges
        bloomColor = color * emissiveMask;
    } else {
        bloomColor = applySoftThreshold(color, pc.threshold, pc.softKnee);
    }

    // Firefly clamp (keeps the hue, scales only the brightness)
    float brightness = maxComponent(bloomColor);
    bloomColor *= min(1.0, kMaxBrightness / max(brightness, 1e-5));

    outColor = vec4(bloomColor, 1.0);
}
