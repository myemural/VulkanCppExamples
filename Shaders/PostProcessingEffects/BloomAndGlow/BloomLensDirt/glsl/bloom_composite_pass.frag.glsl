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
layout(set = 0, binding = 1) uniform sampler2D uBloomImage;
layout(set = 0, binding = 2) uniform sampler2D uLensDirtTexture;

layout(push_constant) uniform CompositePushConstants {
    float exposure;               // Manual exposure in EV
    float bloomStrength;          // Linear interpolation factor between the scene color and bloom
    float bloomNormalization;     // 1 / kBloomMipCount, keeps the summed mip chain energy conserving
    float lensDirtIntensity;      // Multiplier of the extra bloom scattered by the lens dirt (0 means no lens dirt)
    uint isBloomEnabled;          // 0: Bloom disabled (only tone mapping), 1: Bloom enabled
    uint debugView;               // 0: Final image, 1: Bloom only
} pc;

const uint DEBUG_VIEW_FINAL = 0U;
const uint DEBUG_VIEW_BLOOM_ONLY = 1U;

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

void main()
{
    vec2 resolution = vec2(textureSize(gLightPassOutput, 0));
    vec2 uv = gl_FragCoord.xy / resolution;

    vec3 sceneColor = texture(gLightPassOutput, uv).rgb;

    // Bloom mip 0 is half-res, bilinear filtering upsamples it
    // Normalization turns the sum of the mip levels into their average
    vec3 bloomColor = texture(uBloomImage, uv).rgb * pc.bloomNormalization;

    // Lens dirt: Dust and smudged on the lens scatter more of the incoming light
    // The dirt texture only modulates the bloom, so th dirt becomes visible only around bright areas of the screen
    vec3 lensDirt = texture(uLensDirtTexture, uv).rgb;
    bloomColor += bloomColor * lensDirt * pc.lensDirtIntensity;

    vec3 hdrColor = sceneColor;
    if (pc.debugView == DEBUG_VIEW_BLOOM_ONLY) {
        hdrColor = bloomColor;
    } else if (pc.isBloomEnabled != 0U) {
        hdrColor = mix(sceneColor, bloomColor, pc.bloomStrength);
    }

    // Manual exposure control (applied after the bloom, so bloom doesn't depend on exposure)
    hdrColor *= exp2(pc.exposure);

    // Apply ACES tonemapping (HDR to LDR)
    outColor = vec4(tonemapAces(hdrColor), 1.0);
}
