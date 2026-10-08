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

layout(push_constant) uniform VignettePushConstants {
    vec3 vignetteColor;      // Color of the vignette (black gives classic lens darkening)
    float exposure;          // Manual exposure in EV
    vec2 vignetteCenter;     // Center of the vignette in UV space, (0.5, 0.5) is the screen center
    float intensity;         // Amount of the vignette, range [0, 1]
    float smoothness;        // Softness of the vignette borders, range [0.01, 1]
    float roundness;         // 0: More squared shape, 1: Fully round shape, range [0, 1]
    uint isVignetteRounded;  // 0: Shape follows the screen aspect ration (oval), 1: Perfectly circular
    uint isVignetteEnabled;  // 0: Vignette disabled (only tone mapping), 1: Vignette enabled
} pc;

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

// Calculates the vignette factor for given UV (1.0: No darkening, 0.0: Full vignette color)
float calculateVignetteFactor(vec2 uv, vec2 resolution)
{
    // Map user-friendly [0, 1] parameters to the ranges used by the falloff formula
    float intensity = clamp(pc.intensity, 0.0, 1.0) * 3.0;
    float smoothness = max(clamp(pc.smoothness, 0.0, 1.0) * 5.0, 1e-3);
    float roundnessExp = mix(6.0, 1.0, clamp(pc.roundness, 0.0, 1.0)); // Low roundness -> high exponent -> squared

    // Distance to the center for each axis
    vec2 d = abs(uv - pc.vignetteCenter) * intensity;

    // If rounded, correct the horizontal distance with the aspect ratio, so the shape is a perfect circle
    float aspectRatio = resolution.x / resolution.y;
    d.x *= (pc.isVignetteRounded != 0U) ? aspectRatio : 1.0;

    // Higher exponent pushes the falloff towards the corners, which makes the shape more squared
    d = pow(clamp(d, 0.0, 1.0), vec2(roundnessExp));

    return pow(clamp(1.0 - dot(d, d), 0.0, 1.0), smoothness);
}

void main()
{
    // Sampling the HDR output of the light pass
    vec2 resolution = vec2(textureSize(gLightPassOutput, 0));
    vec2 uv = gl_FragCoord.xy / resolution;
    vec3 hdrColor = texture(gLightPassOutput, uv).xyz;

    // Manual exposure control
    vec3 exposedColor = hdrColor * exp2(pc.exposure);

    // Vignette is applied in linear HDR space (before tonemapping), like a real lens light falloff
    // Color is blended towards the vignette color instead of only darkening it
    if (pc.isVignetteEnabled != 0U) {
        float vignetteFactor = calculateVignetteFactor(uv, resolution);
        exposedColor *= mix(pc.vignetteColor, vec3(1.0), vignetteFactor);
    }

    // Apply ACES tonemapping (HDR to LDR)
    outColor = vec4(tonemapAces(exposedColor), 1.0);
}
