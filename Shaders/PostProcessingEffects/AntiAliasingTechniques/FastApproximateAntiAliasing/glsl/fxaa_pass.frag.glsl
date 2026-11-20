#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D gFxaaInput; // rgb = Tona mapped display color, g = Luma

layout(push_constant) uniform FxaaPushConstants {
    uint isFxaaEnabled; // 0: FXAA disabled, 1: FXAA enabled
    uint qualityPreset; // 0: Low, 1: Medium, 2: High, 3: Extreme
} pc;

// Quality presets (values of the FXAA 3.11 presets 12, 23, 29 and 39)
const int kMaxSearchSteps = 12;

struct FxaaPreset
{
    int stepCount;                // Search steps in each direction along the edge
    float steps[kMaxSearchSteps]; // Step sizes in pixels (larger steps for long edges)
    float edgeThreshold;          // Minimum local contrast relative to the max luma
    float edgeThresholdMin;       // Minimum local contrast in dark areas
    float subpixelQuality;        // Amount of the sub-pixel aliasing removal (softness)
};

const FxaaPreset kFxaaPresets[4] = FxaaPreset[](
    // Low (preset 12)
    FxaaPreset(5, float[](1.0, 1.5, 2.0, 4.0, 12.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0), 0.25, 0.0833, 0.75),
    // Medium (preset 23)
    FxaaPreset(6, float[](1.0, 1.5, 2.0, 2.0, 2.0, 8.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0), 0.166, 0.0833, 0.75),
    // High (preset 29)
    FxaaPreset(12, float[](1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0), 0.125, 0.0625, 0.75),
    // Extreme (preset 39)
    FxaaPreset(12, float[](1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0), 0.063, 0.0312, 1.0)
);

// Transforms from sRGB color to linear RGB color
vec3 srgbToLinear(vec3 c)
{
    vec3 lo = c / 12.92;
    vec3 hi = pow((c + 0.055) / 1.055, vec3(2.4));
    return mix(hi, lo, lessThanEqual(c, vec3(0.04045)));
}

float lumaAt(vec2 uv)
{
    return textureLod(gFxaaInput, uv, 0.0).a;
}

// Texel offset must be a compile-time constant, so a macro is used instead of a function
#define lumaAtOffset(uv, offset) textureLodOffset(gFxaaInput, uv, 0.0, offset).a

vec3 fxaa(vec2 uv, vec2 texelSize, FxaaPreset preset)
{
    vec4 centerSample = textureLod(gFxaaInput, uv, 0.0);

    // 1) Local contrast in the cross neighborhood (N: up, S: down, W: left, E: right)
    float lumaM = centerSample.a;
    float lumaN = lumaAtOffset(uv, ivec2(0, -1));
    float lumaS = lumaAtOffset(uv, ivec2(0, 1));
    float lumaW = lumaAtOffset(uv, ivec2(-1, 0));
    float lumaE = lumaAtOffset(uv, ivec2(1, 0));

    float rangeMax = max(max(max(lumaN, lumaS), max(lumaW, lumaE)), lumaM);
    float rangeMin = min(min(min(lumaN, lumaS), min(lumaW, lumaE)), lumaM);
    float range = rangeMax - rangeMin;

    // Not an edge: Contrast is lower than the threshold (relative to the brightness, with a minimum for dark areas)
    if (range < max(preset.edgeThresholdMin, rangeMax * preset.edgeThreshold)) {
        return centerSample.rgb;
    }

    // Corners of the 3x3 neighborhood
    float lumaNW = lumaAtOffset(uv, ivec2(-1, -1));
    float lumaNE = lumaAtOffset(uv, ivec2(1, -1));
    float lumaSW = lumaAtOffset(uv, ivec2(-1, 1));
    float lumaSE = lumaAtOffset(uv, ivec2(1, 1));

    // 2) Edge direction: Second derivates of the luma in the horizontal and vertical directions
    // A horizontal edge has a large vertical change
    float edgeHorizontal = abs(lumaNW - 2.0 * lumaW + lumaSW) + 2.0 * abs(lumaN - 2.0 * lumaM + lumaS) +
                           abs(lumaNE - 2.0 * lumaE + lumaSE);
    float edgeVertical = abs(lumaNW - 2.0 * lumaN + lumaNE) + 2.0 * abs(lumaW - 2.0 * lumaM + lumaE) +
                         abs(lumaSW - 2.0 * lumaS + lumaSE);
    bool isHorizontal = edgeHorizontal >= edgeVertical;

    // Neighbors across the edge (negative and positive side), and the step size across the edge
    float lumaNegative = isHorizontal ? lumaN : lumaW;
    float lumaPositive = isHorizontal ? lumaS : lumaE;
    float stepLength = isHorizontal ? texelSize.y : texelSize.x;

    // Side of the edge: The neighbor with the larger gradient is on the other side of the edge
    float gradientNegative = lumaNegative - lumaM;
    float gradientPositive = lumaPositive - lumaM;
    bool isNegativeSide = abs(gradientNegative) >= abs(gradientPositive);
    float gradientScaled = 0.25 * max(abs(gradientNegative), abs(gradientPositive));
    if (isNegativeSide) {
        stepLength = -stepLength;
    }

    // Average luma of the edge (between the center and the neighbor on the other side)
    float lumaLocalAverage = 0.5 * ((isNegativeSide ? lumaNegative : lumaPositive) + lumaM);

    // 3) Follow the edge in both directions, on the line betweenthe two sides of the edge (half texel offset)
    vec2 edgeUv = uv;
    vec2 edgeStep = isHorizontal ? vec2(texelSize.x, 0.0) : vec2(0.0, texelSize.y);
    if (isHorizontal) {
        edgeUv.y += 0.5 * stepLength;
    } else {
        edgeUv.x += 0.5 * stepLength;
    }

    vec2 uvNegative = edgeUv - edgeStep * preset.steps[0];
    vec2 uvPositive = edgeUv + edgeStep * preset.steps[0];
    float lumaEndNegative = lumaAt(uvNegative) - lumaLocalAverage;
    float lumaEndPositive = lumaAt(uvPositive) - lumaLocalAverage;
    bool isDoneNegative = abs(lumaEndNegative) >= gradientScaled;
    bool isDonePositive = abs(lumaEndPositive) >= gradientScaled;

    // End of the edge: The luma (bilinear sample between both sides) differs from the edge average
    for (int i = 1; i < preset.stepCount && !(isDoneNegative && isDonePositive); ++i) {
        if (!isDoneNegative) {
            uvNegative -= edgeStep * preset.steps[i];
            lumaEndNegative = lumaAt(uvNegative) - lumaLocalAverage;
            isDoneNegative = abs(lumaEndNegative) >= gradientScaled;
        }
        if (!isDonePositive) {
            uvPositive += edgeStep * preset.steps[i];
            lumaEndPositive = lumaAt(uvPositive) - lumaLocalAverage;
            isDonePositive = abs(lumaEndPositive) >= gradientScaled;
        }
    }

    // Distances to the both ends of the edge
    float distanceNegative = isHorizontal ? (uv.x - uvNegative.x) : (uv.y - uvNegative.y);
    float distancePositive = isHorizontal ? (uvPositive.x - uv.x) : (uvPositive.y - uv.y);
    bool isNegativeCloser = distanceNegative < distancePositive;
    float closestDistance = min(distanceNegative, distancePositive);
    float edgeLength = distanceNegative + distancePositive;

    // Offset is used only if the closest and turns towards the side of the center pixel (staircase step)
    // Otherwise, the pixel is not on the blended side of the edge
    bool isCenterDarker = (lumaM - lumaLocalAverage) < 0.0;
    bool isGoodSpan = ((isNegativeCloser ? lumaEndNegative : lumaEndPositive) < 0.0) != isCenterDarker;
    float edgeOffset = isGoodSpan ? (0.5 - closestDistance / edgeLength) : 0.0;

    // 4) Sub-pixel offset: Difference between the center and the low-pass filtered luma of the 3x3 neighborhood
    float lumaAverage = (2.0 * (lumaN + lumaS + lumaW + lumaE) + (lumaNW + lumaNE + lumaSW + lumaSE)) / 12.0;
    float subpixel = clamp(abs(lumaAverage - lumaM) / range, 0.0, 1.0);
    subpixel = (-2.0 * subpixel + 3.0) * subpixel * subpixel; // Smoothstep
    float subpixelOffset = subpixel * subpixel * preset.subpixelQuality;

    // 5) Sample with the larger offset perpendicular to the edge
    float finalOffset = max(edgeOffset, subpixelOffset);
    vec2 finalUv = uv;
    if (isHorizontal) {
        finalUv.y += finalOffset * stepLength;
    } else {
        finalUv.x += finalOffset * stepLength;
    }

    return textureLod(gFxaaInput, finalUv, 0.0).rgb;
}

void main()
{
    vec2 resolution = vec2(textureSize(gFxaaInput, 0));
    vec2 texelSize = 1.0 / resolution;
    vec2 uv = gl_FragCoord.xy * texelSize;

    vec3 displayColor;
    if (pc.isFxaaEnabled != 0U) {
        displayColor = fxaa(uv, texelSize, kFxaaPresets[min(pc.qualityPreset, 3U)]);
    } else {
        displayColor = textureLod(gFxaaInput, uv, 0.0).rgb;
    }

    outColor = vec4(srgbToLinear(displayColor), 1.0);
}
