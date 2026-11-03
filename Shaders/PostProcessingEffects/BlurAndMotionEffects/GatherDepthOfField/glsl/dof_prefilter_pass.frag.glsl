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
layout(set = 0, binding = 1) uniform sampler2D gPosition;

layout(push_constant) uniform BloomPassPushConstants {
    vec2 srcTexelSize;   // 1.0 / source image size
    vec2 dstTexelSize;   // 1.0 / target mip size
    float focusDistance; // Focus distance in meters (view-space units)
    float cocScale;      // CoC radius (in half-res pixels) of an object at infinity
    float maxCocRadius;  // Maximum CoC radius in half-res pixels
} pc;

// Pixels without geometry (cleared G-Buffer) are treated as very far away
const float kBackgroundDepth = 10000.0;

float linearDepth(vec4 positionView)
{
    // gPosition keeps the view-space position, w = 0 means there is no geometry
    return positionView.w > 0.0 ? max(-positionView.z, 1e-3) : kBackgroundDepth;
}

// Thin lens CoC: c = (f^2 / (N * (5 - f))) * (z - S) / z
// The constant part is calculated on CPU (cocScale), so the CoC is "cocScale * (1 - S / z)"
float calculateCoc(float depth)
{
    float coc = pc.cocScale * (1.0 - pc.focusDistance / depth);
    return clamp(coc, -pc.maxCocRadius, pc.maxCocRadius);
}

float maxComponent(vec3 color)
{
    return max(color.r, max(color.g, color.b));
}

void main()
{
    // Each half-res pixel covers a 2x2 block of full-res pixels
    ivec2 baseCoord = ivec2(gl_FragCoord.xy) * 2;
    ivec2 maxCoord = textureSize(gLightPassOutput, 0) - 1;

    const ivec2 offsets[4] = ivec2[](ivec2(0, 0), ivec2(1, 0), ivec2(0, 1), ivec2(1, 1));

    vec3 colorSum = vec3(0.0);
    float weightSum = 0.0;
    float cocMin = pc.maxCocRadius;
    float cocMax = -pc.maxCocRadius;

    for (int i = 0; i < 4; ++i) {
        ivec2 coord = min(baseCoord + offsets[i], maxCoord);
        vec3 color = texelFetch(gLightPassOutput, coord, 0).rgb;
        float coc = calculateCoc(linearDepth(texelFetch(gPosition, coord, 0)));

        // CoC weight: In-focus pixels don't leak into the blurred ones
        // Luma weight (like Karis average): Small and very bright pixels don't flicker
        float weight = abs(coc) / (maxComponent(color) + 1.0);
        colorSum += color * weight;
        weightSum += weight;

        cocMin = min(cocMin, coc);
        cocMax = max(cocMax, coc);
    }

    vec3 color = colorSum / max(weightSum, 1e-5);

    // Keep the CoC with the largest magnitude, so near field silhouttes don't shrink in half resolution
    float coc = (-cocMin > cocMax) ? cocMin : cocMax;

    // Fade out the color of in-focus pixels, they are taken from the full-res image in the composite pass
    color *= smoothstep(0.0, 1.0, abs(coc));
    outColor = vec4(color, coc);
}
