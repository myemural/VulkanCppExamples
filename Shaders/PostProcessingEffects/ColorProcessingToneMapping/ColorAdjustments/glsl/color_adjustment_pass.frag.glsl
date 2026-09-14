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

layout(push_constant) uniform ColorAdjustmentPushConstants {
    float brightness;
    float contrast;
    float saturation;
    float hueShift;
    float colorTemperature;
    float whiteBalance;
} pc;

vec3 applyWhiteBalanceAndTemperature(vec3 color, float temperature, float tint)
{
    // Temperature axis: Negative = Cold (Blue), Positive = Warm (Orange)
    vec3 warmShift = vec3(0.15, 0.05, -0.15) * temperature;
    // White balance / tint axis: Negative = Green, Positive = Magenta
    vec3 tintShift = vec3(0.05, -0.1, 0.05) * tint;
    return color + warmShift + tintShift;
}

vec3 applyBrightness(vec3 color, float brightness)
{
    return color + brightness;
}

vec3 applyContrast(vec3 color, float contrast)
{
    return (color - 0.5) * contrast + 0.5;
}

vec3 applySaturation(vec3 color, float saturation)
{
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    return mix(vec3(luminance), color, saturation);
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 applyHueShift(vec3 color, float hueDegrees)
{
    vec3 hsv = rgb2hsv(color);
    hsv.x = fract(hsv.x + hueDegrees / 360.0);
    return hsv2rgb(hsv);
}

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gLightPassOutput, 0));
    vec3 sceneColor = texture(gLightPassOutput, uv).rgb;

    // Order is important here!
    sceneColor = applyWhiteBalanceAndTemperature(sceneColor, pc.colorTemperature, pc.whiteBalance);
    sceneColor = applyBrightness(sceneColor, pc.brightness);
    sceneColor = applyContrast(sceneColor, pc.contrast);
    sceneColor = applySaturation(sceneColor, pc.saturation);
    sceneColor = applyHueShift(sceneColor, pc.hueShift);

    outColor = vec4(clamp(sceneColor, 0.0, 1.0), 1.0);
}
