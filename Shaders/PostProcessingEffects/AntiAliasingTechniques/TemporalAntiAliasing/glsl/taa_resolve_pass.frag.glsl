#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D gCurrentColor; // HDR color of the current (jittered) frame
layout(set = 0, binding = 1) uniform sampler2D gHistory;      // Resolved HDR color of the previous frame
layout(set = 0, binding = 2) uniform sampler2D gVelocity;     // Screen-space velocity in UV units
layout(set = 0, binding = 3) uniform sampler2D gPosition;     // View-space position (w = 0: No geometry)

layout(push_constant) uniform TaaPushConstants {
    float currentFrameWeight;  // Weight of the current frame in the blend
    uint isTaaEnabled;         // 0: TAA disabled (current frame only), 1: TAA enabled
    uint isHistoryValid;       // 0: History is reset (first frame or TAA re-enabled), 1: History is valid
} pc;

ivec2 gImageSize;

// Reversible color compression: Bright pixels (HDR) don't dominate the neighborhood clamp and the blend
vec3 compressColor(vec3 color)
{
    return color / (1.0 + max(color.r, max(color.g, color.b)));
}

vec3 decompressColor(vec3 color)
{
    return color / max(1.0 - max(color.r, max(color.g, color.b)), 1e-4);
}

vec3 fetchCurrent(ivec2 position)
{
    return texelFetch(gCurrentColor, clamp(position, ivec2(0), gImageSize - 1), 0).rgb;
}

// Catmull-Rom filtered history with 5 bilinear fetches
// Reference: https://research.activision.com/publications/archives/filmic-smaasharp-morphological-and-temporal-antialiasing
vec3 sampleHistoryCatmullRom(vec2 uv)
{
    vec2 resolution = vec2(gImageSize);
    vec2 samplePosition = uv * resolution;
    vec2 texelCenter = floor(samplePosition - 0.5) + 0.5;
    vec2 f = samplePosition - texelCenter;

    // Catmull-Rom weights of the 4 texels on each axis
    vec2 w0 = f * (-0.5 + f * (1.0 - 0.5 * f));
    vec2 w1 = 1.0 + f * f * (-2.5 + 1.5 * f);
    vec2 w2 = f * (0.5 + f * (2.0 - 1.5 * f));
    vec2 w3 = f * f * (-0.5 + 0.5 * f);

    // The two middle texels are read with one bilinear fetch
    vec2 w12 = w1 + w2;
    vec2 offset12 = w2 / w12;

    vec2 uv0 = (texelCenter - 1.0) / resolution;
    vec2 uv3 = (texelCenter + 2.0) / resolution;
    vec2 uv12 = (texelCenter + offset12) / resolution;

    vec3 result = vec3(0.0);
    result += textureLod(gHistory, vec2(uv12.x, uv0.y), 0.0).rgb * (w12.x * w0.y);
    result += textureLod(gHistory, vec2(uv0.x, uv12.y), 0.0).rgb * (w0.x * w12.y);
    result += textureLod(gHistory, vec2(uv12.x, uv12.y), 0.0).rgb * (w12.x * w12.y);
    result += textureLod(gHistory, vec2(uv3.x, uv12.y), 0.0).rgb * (w3.x * w12.y);
    result += textureLod(gHistory, vec2(uv12.x, uv3.y), 0.0).rgb * (w12.x * w3.y);

    float weightSum = w12.x * w0.y + w0.x * w12.y + w12.x * w12.y + w3.x * w12.y + w12.x * w3.y;
    return max(result / weightSum, 0.0); // Catmull-Rom has negative lobes
}

void main()
{
    gImageSize = textureSize(gCurrentColor, 0);
    ivec2 position = ivec2(gl_FragCoord.xy);
    vec2 uv = gl_FragCoord.xy / vec2(gImageSize);

    vec3 currentColor = fetchCurrent(position);
    if (pc.isTaaEnabled == 0U || pc.isHistoryValid == 0U) {
        outColor = vec4(currentColor, 1.0);
        return;
    }

    // 3x3 neighborhood: Color box of the current frame and the closest pixel (for the velocity)
    vec3 compressedCurrent = compressColor(currentColor);
    vec3 neighborhoodMin = compressedCurrent;
    vec3 neighborhoodMax = compressedCurrent;
    ivec2 closestPosition = position;
    float closestDepth = -1e30;

    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            ivec2 samplePosition = clamp(position + ivec2(x, y), ivec2(0), gImageSize - 1);

            vec3 neighborColor = compressColor(texelFetch(gCurrentColor, samplePosition, 0).rgb);
            neighborhoodMin = min(neighborhoodMin, neighborColor);
            neighborhoodMax = max(neighborhoodMax, neighborColor);

            // View-space Z is negative, the closest pixel has the largest Z (background is the farthest)
            vec4 viewPosition = texelFetch(gPosition, samplePosition, 0);
            float depth = viewPosition.w > 0.0 ? viewPosition.z : -1e20;
            if (depth > closestDepth) {
                closestDepth = depth;
                closestPosition = samplePosition;
            }
        }
    }

    // 1) Reprojection: Velocity of the closest pixel keeps the edges of the moving objects sharp
    vec2 velocity = texelFetch(gVelocity, closestPosition, 0).xy;
    vec2 previousUv = uv - velocity;

    // Disocclusion at the screen borders: No history outside of the screen
    if (any(lessThan(previousUv, vec2(0.0))) || any(greaterThan(previousUv, vec2(1.0)))) {
        outColor = vec4(currentColor, 1.0);
        return;
    }
    vec3 history = compressColor(sampleHistoryCatmullRom(previousUv));

    // 2) Neighborhood clamp: History color wihch cannot be in the current neighborhood are invalid (ghosting)
    history = clamp(history, neighborhoodMin, neighborhoodMax);

    // 3) Exponential moving average
    vec3 result = mix(history, compressedCurrent, pc.currentFrameWeight);

    outColor = vec4(decompressColor(result), 1.0);
}
