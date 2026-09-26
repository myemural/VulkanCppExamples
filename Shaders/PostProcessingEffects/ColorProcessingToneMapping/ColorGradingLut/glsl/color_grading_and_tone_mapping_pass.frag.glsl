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

layout(set = 0, binding = 1) uniform sampler3D gColorGradingLut;

layout(push_constant) uniform PostProcessingPushConstants {
    float exposure; // Manual exposure level
} pc;

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

vec3 linearToGamma(vec3 color)
{
    return pow(max(color, vec3(0.0)), vec3(1.0 / 2.2));
}

vec3 gammaToLinear(vec3 color)
{
    return pow(max(color, vec3(0.0)), vec3(2.2));
}

void main()
{
    vec3 hdrColor = texelFetch(gLightPassOutput, ivec2(gl_FragCoord.xy), 0).rgb;

    // Manual exposure control
    vec3 exposedColor = hdrColor * exp2(pc.exposure);

    // Tonemap HDR to LDR
    vec3 tonemappedColor = agxEotf(tonemapAgx(exposedColor));

    // Encode to gamma space
    vec3 gammaColor = linearToGamma(tonemappedColor);

    // Sample the stylized look LUT (trilinear)
    vec3 gradedGammaColor = texture(gColorGradingLut, gammaColor).rgb;

    // Back to scene-linear so the sRGB-format attachment encodes correctly on store
    outColor = vec4(gammaToLinear(gradedGammaColor), 1.0);
}
