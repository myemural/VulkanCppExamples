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

layout(push_constant) uniform ColorCorrectionPushConstants {
    float brightness;            // Range: [-0.5, 0.5], Default 0.0 (additive offset)
    float contrast;              // Range: [0.0, 2.0], Default 1.0
    float saturation;            // Range:[0.0, 2.0], Default 1.0
    float hueShift;              // Range: [-180, 180], default 0.0 (in degrees)
    uint applyInDisplayEncoding; // 0 = Linear, 1 = sRGB-encoded space
} pc;

// Rec.709 luma coefficients
const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);

// Mid-gray pivot for contrast
const float kContrastPivotEncoded = 0.5;
const float kContrastPivotLinear = 0.2;

// Transforms from linear RGB color to sRGB color
vec3 linearToSrgb(vec3 c)
{
    vec3 lo = c * 12.92;
    vec3 hi = 1.055 * pow(max(c, vec3(1.0e-8)), vec3(1.0 / 2.4)) - 0.055;
    return mix(hi, lo, lessThanEqual(c, vec3(0.0031308)));
}

// Transforms from sRGB color to linear RGB color
vec3 srgbToLinear(vec3 c)
{
    vec3 lo = c / 12.92;
    vec3 hi = pow((c + 0.055) / 1.055, vec3(2.4));
    return mix(hi, lo, lessThanEqual(c, vec3(0.04045)));
}

// Apply brightness with additive offset
vec3 applyBrightness(vec3 color, float brightness)
{
    return color + brightness;
}

// Scale around a fixed mid-gray pivot so that the pivot value is preserved
vec3 applyContrast(vec3 color, float contrast, float pivot)
{
    return (color - pivot) * contrast + pivot;
}

// Interpolate between achromatic (luma) version of the pixel and the pixel itself
vec3 applySaturation(vec3 color, float saturation)
{
    float luminance = dot(color, kLumaRec709);
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

// Rotate the hue angle, requires non-negative input, hence the clamp should be applied before call it
vec3 applyHueShift(vec3 color, float hueDegrees)
{
    vec3 hsv = rgb2hsv(color);
    hsv.x = fract(hsv.x + hueDegrees / 360.0);
    return hsv2rgb(hsv);
}

void main()
{
    vec3 sceneColor = texelFetch(gLightPassOutput, ivec2(gl_FragCoord.xy), 0).rgb;

    bool isEncoded = pc.applyInDisplayEncoding == 1U;

    vec3 color = clamp(sceneColor, 0.0, 1.0);
    if (isEncoded) {
        color = linearToSrgb(color);
    }

    // Order is important here, tonal first, then chromatic
    color = applyBrightness(color, pc.brightness);
    color = applyContrast(color, pc.contrast, isEncoded ? kContrastPivotEncoded : kContrastPivotLinear);
    color = clamp(color, 0.0, 1.0);

    color = applySaturation(color, pc.saturation);
    color = clamp(color, 0.0, 1.0);

    color = applyHueShift(color, pc.hueShift);
    color = clamp(color, 0.0, 1.0);

    if (isEncoded) {
        color = srgbToLinear(color);
    }

    outColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
