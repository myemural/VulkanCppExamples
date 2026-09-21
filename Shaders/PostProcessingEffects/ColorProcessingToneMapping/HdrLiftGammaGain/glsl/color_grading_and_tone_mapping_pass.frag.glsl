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
    vec4 lift;      // rgb = Color in Linear Rec.709, a = Level Offset in [-1, 1]
    vec4 gamma;     // rgb = Color in Linear Rec.709, a = Level Offset in [-1, 1]
    vec4 gain;      // rgb = Color in Linear Rec.709, a = Level Offset in [-1, 1]
    float exposure; // Manual exposure level
} pc;

// Rec.709 luma coefficients
const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);

// How much hue each preset color injects
// Lift is kept small because it is added directly to scene-linear values, where the shadows sit very close to zero
const float kLiftColorScale = 0.15;
const float kGammaColorScale = 0.8;
const float kGainColorScale = 0.8;

// Lift is additive, so its offset stays linear
const float kLiftOffsetScale = 0.1;

// Guards the exponent, without this an offset near the end of its range can drive "1 / gammaTerm" high enough to
// overflow a float on bright HDR pixels
const vec2 kGammaTermRange = vec2(0.25, 4.0);

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

// Strip the level out of a  preset color, so only its hue survives
vec3 hueOnly(vec3 color, float scale)
{
    vec3 tinted = color * scale;
    return tinted - dot(tinted, kLumaRec709);
}

vec3 applyLiftGammaGain(vec3 color)
{
    // Additive, biased toward the shadows
    vec3 liftTerm = hueOnly(pc.lift.rgb, kLiftColorScale) + pc.lift.a * kLiftOffsetScale;

    // Exponent, biased toward the midtones
    vec3 gammaTerm = hueOnly(pc.gamma.rgb, kGammaColorScale) + exp2(pc.gamma.a);
    gammaTerm = clamp(gammaTerm, kGammaTermRange.x, kGammaTermRange.y);
    vec3 invGamma = 1.0 / gammaTerm;

    // Multiplicative, biased toward the highlights
    vec3 gainTerm = hueOnly(pc.gain.rgb, kGainColorScale) + exp2(pc.gain.a);
    gainTerm = max(gainTerm, vec3(0.0));

    vec3 graded = color * gainTerm + liftTerm;

    // A negative lift can push channels below zero, mirroring the power around the origin keeps the curve continuous
    // there instead of producing NaN from pow() on a negative base
    return sign(graded) * pow(abs(graded), invGamma);
}

vec3 agxDefaultContrastApprox(vec3 x)
{
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    return 15.5 * x4 * x2 - 40.14 * x4 * x + 31.96 * x4 - 6.868 * x2 * x + 0.4298 * x2 + 0.1191 * x - 0.00232;
}

// AgX tonemapper
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

    return clamp(kAgXOutsetMatrix * val, 0.0, 1.0);
}

// Linearize the display-encoded AgX output
vec3 agxEotf(vec3 color)
{
    return pow(max(color, vec3(0.0)), vec3(2.2));
}

void main()
{
    vec3 hdrColor = texelFetch(gLightPassOutput, ivec2(gl_FragCoord.xy), 0).rgb;

    hdrColor = applyLiftGammaGain(hdrColor);

    // The adaptation can push saturated colors outside the Rec.709 gamut
    // Clipping to zero is the simplest solution, but real pipeline would gamut compress instead
    hdrColor = max(hdrColor, vec3(0.0));

    // Manual exposure control
    vec3 exposedColor = hdrColor * exp2(pc.exposure);

    // Apply AgX tonemapper
    outColor = vec4(agxEotf(tonemapAgx(exposedColor)), 1.0);
}
