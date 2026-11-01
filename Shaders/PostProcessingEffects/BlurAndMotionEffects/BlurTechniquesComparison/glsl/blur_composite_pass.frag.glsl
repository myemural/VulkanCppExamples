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
layout(set = 0, binding = 1) uniform sampler2D uBlurImageA;
layout(set = 0, binding = 2) uniform sampler2D uBlurImageB;

layout(push_constant) uniform CompositePushConstants {
    float exposure;     // Manual exposure in EV
    uint resultImage;   // 0: Scene color (no blur), 1: Blur image A, 2: Blur image B
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

void main()
{
    vec2 resolution = vec2(textureSize(gLightPassOutput, 0));
    vec2 uv = gl_FragCoord.xy / resolution;

    // Result of the last blur pass is in image A or B (depends on the technique and the iteration count)
    vec3 hdrColor;
    if (pc.resultImage == 1U) {
        hdrColor = texture(uBlurImageA, uv).rgb;
    } else if (pc.resultImage == 2U) {
        hdrColor = texture(uBlurImageB, uv).rgb;
    } else {
        hdrColor = texture(gLightPassOutput, uv).rgb;
    }

    // Apply ACES tonemapping (HDR to LDR)
    outColor = vec4(tonemapAces(hdrColor * exp2(pc.exposure)), 1.0);
}
