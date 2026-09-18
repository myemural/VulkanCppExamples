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

layout(push_constant) uniform ToneMappingPushConstants {
    float exposure;
    // Tone mapping mode for output
    // 0: Off (raw exposed HDR, clamped to [0,1] by the render target)
    // 1: Reinhard (simple)
    // 2: ACES (Narkowicz fit)
    // 3: Uchimura (Gran Turismo style)
    // 4: AgX (fast approximation)
    // 5: Parametric Filmic (Hable / Uncharted2 style)
    uint toneMappingMode;
} pc;

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

vec3 tonemapFilmicCurve(vec3 x, float shoulderStrength, float linearStrength, float linearAngle,
        float toeStrength, float toeNumerator, float toeDenominator)
{
    return ((x * (shoulderStrength * x + linearAngle * linearStrength) + toeStrength * toeNumerator) /
    (x * (shoulderStrength * x + linearStrength) + toeStrength * toeDenominator)) -
    toeNumerator / toeDenominator;
}

// Parametric Filmic (Hable / Uncharted2 style)
// Reference: https://www.gdcvault.com/play/1012459/Uncharted-2-HDR
vec3 tonemapParametricFilmic(vec3 color)
{
    /// TODO: These values can be transferred via push constants later.
    const float shoulderStrength = 0.22;
    const float linearStrength = 0.30;
    const float linearAngle = 0.10;
    const float toeStrength = 0.20;
    const float toeNumerator = 0.01;
    const float toeDenominator = 0.30;
    const float linearWhite = 11.2; // Scene-linear value that should map to 1.0 (pure white)

    vec3 curved = tonemapFilmicCurve(color, shoulderStrength, linearStrength, linearAngle, toeStrength,
            toeNumerator, toeDenominator);
    vec3 whiteScale = vec3(1.0) / tonemapFilmicCurve(vec3(linearWhite), shoulderStrength, linearStrength,
            linearAngle, toeStrength, toeNumerator, toeDenominator);
    return clamp(curved * whiteScale, 0.0, 1.0);
}

void main()
{
    // Sampling the HDR output of the light pass
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gLightPassOutput, 0));
    vec3 hdrColor = texture(gLightPassOutput, uv).xyz;

    // Manual exposure control
    vec3 exposedColor = hdrColor * exp2(pc.exposure);

    if (pc.toneMappingMode == 1) {
        outColor = vec4(tonemapReinhard(exposedColor), 1.0);
    } else if (pc.toneMappingMode == 2) {
        outColor = vec4(tonemapAces(exposedColor), 1.0);
    } else if (pc.toneMappingMode == 3) {
        outColor = vec4(tonemapUchimura(exposedColor), 1.0);
    } else if (pc.toneMappingMode == 4) {
        outColor = vec4(agxEotf(tonemapAgx(exposedColor)), 1.0);
    } else if (pc.toneMappingMode == 5) {
        outColor = vec4(tonemapParametricFilmic(exposedColor), 1.0);
    } else {
        outColor = vec4(exposedColor, 1.0);
    }
}
