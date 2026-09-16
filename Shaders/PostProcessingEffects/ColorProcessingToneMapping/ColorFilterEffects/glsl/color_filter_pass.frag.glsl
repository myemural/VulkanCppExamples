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

layout(push_constant) uniform ColorFilterPushConstants {
    vec4 duoToneDarkColor;
    vec4 duoToneBrightColor;
    // Filter mode for output
    // 0: Color Filters Disabled
    // 1: Grayscale
    // 2: Invert Colors
    // 3: Sepia
    // 4: Duo Tone
    // 5: Channel Swap (RGB to BRG)
    uint filterMode;
} pc;

vec3 grayscale(vec3 color)
{
    // RGB to Luminance conversion
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    return vec3(luminance);
}

vec3 invertColors(vec3 color)
{
    return vec3(1.0) - color;
}

vec3 sepia(vec3 color)
{
    return vec3(dot(color, vec3(0.393, 0.769, 0.189)), dot(color, vec3(0.349, 0.686, 0.168)),
                dot(color, vec3(0.272, 0.534, 0.131)));
}

vec3 duoTone(vec3 color)
{
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));

    // Dark color
    vec3 colorA = pc.duoToneDarkColor.rgb;

    // Bright color
    vec3 colorB = pc.duoToneBrightColor.rgb;

    return mix(colorA, colorB, luminance);
}

vec3 channelSwap(vec3 color)
{
    // RGB to BRG
    return color.brg;
}

void main()
{
    // Sampling G-Buffer
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gLightPassOutput, 0));
    vec3 sceneColor = texture(gLightPassOutput, uv).xyz;

    if (pc.filterMode == 1) {
        outColor = vec4(grayscale(sceneColor), 1.0);
    } else if (pc.filterMode == 2) {
        outColor = vec4(invertColors(sceneColor), 1.0);
    } else if (pc.filterMode == 3) {
        outColor = vec4(sepia(sceneColor), 1.0);
    } else if (pc.filterMode == 4) {
        outColor = vec4(duoTone(sceneColor), 1.0);
    } else if (pc.filterMode == 5) {
        outColor = vec4(channelSwap(sceneColor), 1.0);
    } else {
        outColor = vec4(sceneColor, 1.0);
    }
}
