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

layout(std430, set = 0, binding = 1) readonly buffer AutoExposureBuffer{
    float averageLuminance;    // Adapted luminance (linear)
    float exposureEv;          // Read by the tone mapping pass
    float targetLogLuminance;  // This frame's metered value
    float adaptedLogLuminance; // Persistent adaptation state
} autoExposure;

layout(set = 0, binding = 2) uniform sampler2D gBlurredLogLuminance;

layout(set = 0, binding = 3) uniform sampler3D gBilateralGrid;

layout(push_constant) uniform ToneMappingPushConstants {
    // Tone mapping mode for output
    // 0: Simple Reinhard (Default)
    // 1: ACES (Narkowicz fit)
    // 2: Uchimura (Gran Turismo style)
    // 3: AgX (fast approximation)
    uint toneMappingMode;
    float middleGrey; // Same as the auto exposure key value (0.18)
    float highlightContrastScale; // < 1 compresses bright surroundings
    float shadowContrastScale; // < 1 lifts dark surroundings
    float detailStrength; // 1 keeps local detail, > 1 boosts it
    float blurredLuminanceBlend; // 0 = bilateral grid only, 1 = blurred luminance only
    float gridMinLogLuminance;
    float gridLogLuminanceRange;
    uint localExposureEnabled;
} pc;

const float kLocalExposureDownscaleFactor = 32.0; // Must match with the setup compute shader
const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);
const float kEpsilon = 1e-8;

const mat3 kAgXInsetMatrix = mat3(
        0.856627153315983, 0.137318972929847, 0.11189821299995,
        0.0951212405381588, 0.761241990602591,  0.0767994186031903,
        0.0482516061458583,  0.101439036467562, 0.811302368396859);

const mat3 kAgXOutsetMatrix = mat3(
        1.1271005818144368, -0.1413297634984383,  -0.14132976349843826,
        -0.11060664309660323, 1.157823702216272,   -0.11060664309660294,
        -0.016493938717834573,-0.016493938717834257, 1.2519364065950405);

const float kAgXMinEv = -12.47393;
const float kAgXMaxEv = 4.026069;

// Reinhard (simple)
vec3 tonemapReinhard(vec3 color)
{
    return color / (color + vec3(1.0));
}

// ACES Filmic (Narkowicz fit)
// Reference: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 tonemapAces(vec3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

vec3 tonemapUchimuraCurve(vec3 x, float maxBrightness, float contrast, float linStart, float linLength,
        float black, float pedestal)
{
    float l0 = ((maxBrightness - linStart) * linLength) / contrast;
    float L0 = linStart - linStart / contrast;
    float L1 = linStart + (1.0 - linStart) / contrast;
    float S0 = linStart + l0;
    float S1 = linStart + contrast * l0;
    float C2 = (contrast * maxBrightness) / (maxBrightness - S1);
    float CP = -C2 / maxBrightness;

    vec3 w0 = vec3(1.0 - smoothstep(0.0, linStart, x));
    vec3 w2 = vec3(step(linStart + l0, x));
    vec3 w1 = vec3(1.0 - w0 - w2);

    vec3 T = vec3(linStart * pow(x / linStart, vec3(black)) + pedestal);
    vec3 S = vec3(maxBrightness - (maxBrightness - S1) * exp(CP * (x - S0)));
    vec3 L = vec3(linStart + contrast * (x - linStart));

    return T * w0 + L * w1 + S * w2;
}

// Uchimura (Gran Turismo style)
// Reference: https://www.slideshare.net/slideshow/hdr-theory-and-practicce-jp/79290599
vec3 tonemapUchimura(vec3 color)
{
    /// TODO: These values can be transferred via push constants later.
    const float maxBrightness = 1.0;  // Display peak brightness
    const float contrast = 1.0;       // Contrast in the linear section
    const float linStart = 0.22;      // Start of the linear section
    const float linLength = 0.4;      // Length of the linear section
    const float black = 1.33;         // Toe curve strength (black tightness)
    const float pedestal = 0.0;       // Minimum black pedestal
    return tonemapUchimuraCurve(color, maxBrightness, contrast, linStart, linLength, black, pedestal);
}

vec3 agxDefaultContrastApprox(vec3 x)
{
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    return 15.5 * x4 * x2 - 40.14 * x4 * x + 31.96 * x4 - 6.868 * x2 * x + 0.4298 * x2 + 0.1191 * x - 0.00232;
}

// AgX (fast approximation)
// Returns a display-encoded value, so the result still has to be linearized before it is written to sRGB render target
// Reference: https://iolite-engine.com/blog_posts/minimal_agx_implementation
vec3 tonemapAgx(vec3 color)
{
    vec3 val = kAgXInsetMatrix * color;

    // Log2 encode into the AgX working range
    val = clamp(log2(max(val, vec3(1e-10))), kAgXMinEv, kAgXMaxEv);
    val = (val - kAgXMinEv) / (kAgXMaxEv - kAgXMinEv);

    // Apply the default AgX contrast/look approximation
    val = agxDefaultContrastApprox(val);

    // Undo the inset transform to move back to display-referred values
    val = kAgXOutsetMatrix * val;

    return clamp(val, 0.0, 1.0);
}

// Linearize the display-encoded AgX output
vec3 agxEotf(vec3 color)
{
    return pow(max(color, vec3(0.0)), vec3(2.2));
}

// Bilateral grid slicing: Samples the grid at the pixel's position and its own luminance
// So, only nearby pixels with the similar brightness are treated as "surroundings" (edge-aware)
float sliceBilateralGrid(vec2 gridUv, float pixelLog, float fallbackLog)
{
    const float gridZ = clamp((pixelLog - pc.gridMinLogLuminance) / pc.gridLogLuminanceRange, 0.0, 1.0);
    const vec2 homogeneous = texture(gBilateralGrid, vec3(gridUv, gridZ)).rg;
    return (homogeneous.y > 1e-4) ? homogeneous.x / homogeneous.y : fallbackLog;
}

// Local exposure factor calculation (base/detail decomposition in log2 space)
float calculateLocalExposure(vec3 hdrColor, float globalExposureLog)
{
    const float middleGreyLog = log2(pc.middleGrey);
    const float pixelLog = log2(max(dot(hdrColor, kLumaRec709), kEpsilon)); // Unexposed

    // The low-res inputs cover "ceil(size / 32) * 32" pixels, so map UV accordingly
    const vec2 gridUv = gl_FragCoord.xy / (vec2(textureSize(gBlurredLogLuminance, 0)) * kLocalExposureDownscaleFactor);

    const float blurredLog = texture(gBlurredLogLuminance, gridUv).r;
    const float bilateralLog = sliceBilateralGrid(gridUv, pixelLog, blurredLog);
    const float baseLog = mix(bilateralLog, blurredLog, pc.blurredLuminanceBlend) + globalExposureLog;
    const float exposedPixelLog = pixelLog + globalExposureLog;

    const float contrastScale = (baseLog > middleGreyLog) ? pc.highlightContrastScale : pc.shadowContrastScale;
    const float targetLog = middleGreyLog + (baseLog - middleGreyLog) * contrastScale +
                            (exposedPixelLog - baseLog) * pc.detailStrength;

    return exp2(targetLog - exposedPixelLog);
}

void main()
{
    // Sampling the HDR output of the light pass
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gLightPassOutput, 0));
    vec3 hdrColor = texture(gLightPassOutput, uv).xyz;

    // Global exposure (auto exposure + eye adaptation)
    float globalExposureLog = autoExposure.exposureEv;
    vec3 exposedColor = hdrColor * exp2(globalExposureLog);

    // Local exposure
    if (pc.localExposureEnabled != 0U) {
        exposedColor *= calculateLocalExposure(hdrColor, globalExposureLog);
    }

    if (pc.toneMappingMode == 0) {
        outColor = vec4(tonemapReinhard(exposedColor), 1.0);
    } else if (pc.toneMappingMode == 1) {
        outColor = vec4(tonemapAces(exposedColor), 1.0);
    } else if (pc.toneMappingMode == 2) {
        outColor = vec4(tonemapUchimura(exposedColor), 1.0);
    } else if (pc.toneMappingMode == 3) {
        outColor = vec4(agxEotf(tonemapAgx(exposedColor)), 1.0);
    } else {
        outColor = vec4(tonemapReinhard(exposedColor), 1.0);
    }
}
