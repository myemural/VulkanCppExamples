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

layout(push_constant) uniform BrightPassPushConstants {
    vec2 invOutputSize;         // 1.0 / virtual image size
    float exposure;             // Manual exposure in EV
    float phosphorPersistence;  // Brightness of the previous filed's lines after one frame
    uint isInterlaced;          // 0: Progressive (every line is updated), 1: Interlaced (only the current field)
    uint fieldIndex;            // 0: Even lines, 1: Odd lines
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
    uint line = uint(gl_FragCoord.y);

    // Lines of the corner field: Output black with alpha "1 - persistence"
    if (pc.isInterlaced != 0U && (line % 2U) != pc.fieldIndex) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0 - pc.phosphorPersistence);
        return;
    }

    // Every virtual pixel covers several source pixels, so its area is box filtered with 4 bilinear taps
    vec2 cellCenter = gl_FragCoord.xy * pc.invOutputSize;
    vec2 halfTexel = 0.5 / vec2(textureSize(gLightPassOutput, 0));
    vec2 offsets[4] = vec2[](vec2(-0.25, -0.25), vec2(0.25, -0.25), vec2(-0.25, 0.25), vec2(0.25, 0.25));

    vec3 hdrColor = vec3(0.0);
    for (int i = 0; i < 4; ++i) {
        vec2 sampleUv = clamp(cellCenter + offsets[i] * pc.invOutputSize, halfTexel, 1.0 - halfTexel);
        hdrColor += texture(gLightPassOutput, sampleUv).rgb;
    }
    hdrColor *= 0.25;

    // CRT works on display-referred (tone mapped) liner light, alpha 1 replacec the previous color of the line
    outColor = vec4(tonemapAces(hdrColor * exp2(pc.exposure)), 1.0);
}
