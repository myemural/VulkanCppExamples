#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D gColor;        // Display (sRGB gamma) space color
layout(set = 0, binding = 1) uniform sampler2D gBlendWeights; // Output of the belnding weight calculation
layout(set = 0, binding = 2) uniform sampler2D gEdges;        // Output of the edge detection (for debug view)

layout(push_constant) uniform SmaaPushConstants {
    float threshold;               // Luma difference threshold of the edges
    uint maxSearchSteps;           // Search steps of the horizontal/vertical lines
    uint maxSearchStepsDiag;       // Search steps of the diagonal lines (0: Diagonal detection disabled)
    uint isCornerDetectionEnabled; // 0: Sharp corners are blended, 1: Sharp corners are preserved
    uint isSmaaEnabled;            // 0: SMAA disabled, 1: SMAA enabled
    uint debugView;                // 0: Final image, 1: Edges, 2: Blending weights
} pc;

const uint kDebugViewEdges = 1U;
const uint kDebugViewBlendWeights = 2U;

// Transforms from sRGB color to linear RGB color
vec3 srgbToLinear(vec3 c)
{
    vec3 lo = c / 12.92;
    vec3 hi = pow((c + 0.055) / 1.055, vec3(2.4));
    return mix(hi, lo, lessThanEqual(c, vec3(0.04045)));
}

vec3 neighborhoodBlending(vec2 uv, vec2 texelSize)
{
    // Weights of the four edges of the pixel: Right, bottom, top, left
    vec4 a;
    a.x = textureLod(gBlendWeights, uv + vec2(texelSize.x, 0.0), 0.0).a; // Right
    a.y = textureLod(gBlendWeights, uv + vec2(0.0, texelSize.y), 0.0).g; // Bottom
    a.wz = textureLod(gBlendWeights, uv, 0.0).xz; // Top / Left

    if (dot(a, vec4(1.0)) < 1e-5) {
        return textureLod(gColor, uv, 0.0).rgb;
    }

    // Blend in the direction of the largest weight (horizontal or vertical)
    bool isHorizontal = max(a.x, a.z) > max(a.y, a.w);

    vec4 blendingOffset = isHorizontal ? vec4(a.x, 0.0, a.z, 0.0) : vec4(0.0, a.y, 0.0, a.w);
    vec2 blendingWeight = isHorizontal ? a.xz : a.yw;
    blendingWeight /= dot(blendingWeight, vec2(1.0));

    // Bilinear filtering mixes the pixel with the neighbor on each side
    vec4 blendingUv = blendingOffset * vec4(texelSize, - texelSize) + uv.xyxy;
    vec3 color = blendingWeight.x * textureLod(gColor, blendingUv.xy, 0.0).rgb;
    color += blendingWeight.y * textureLod(gColor, blendingUv.zw, 0.0).rgb;

    return color;
}

void main()
{
    vec2 resolution = vec2(textureSize(gColor, 0));
    vec2 texelSize = 1.0 / resolution;
    vec2 uv = gl_FragCoord.xy * texelSize;

    vec3 displayColor;
    if (pc.debugView == kDebugViewEdges) {
        displayColor = vec3(textureLod(gEdges, uv, 0.0).rg, 0.0);
    } else if (pc.debugView == kDebugViewBlendWeights) {
        displayColor = textureLod(gBlendWeights, uv, 0.0).rgb;
    } else if (pc.isSmaaEnabled != 0U) {
        displayColor = neighborhoodBlending(uv, texelSize);
    } else {
        displayColor = textureLod(gColor, uv, 0.0).rgb;
    }

    outColor = vec4(srgbToLinear(displayColor), 1.0);
}
