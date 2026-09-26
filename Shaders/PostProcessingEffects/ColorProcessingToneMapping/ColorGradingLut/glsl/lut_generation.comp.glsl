#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

// Storage image the LUT is baked into. Must be created with
// VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_VIEW_TYPE_3D,
// dimensions kLutSize x kLutSize x kLutSize.
layout(set = 0, binding = 0, rgba16f) uniform writeonly image3D uColorGradingLut;

layout(push_constant) uniform LutBakePushConstants {
    vec4 shadowTint;    // rgb = Tint color applied to shadows, a = Strength [0,1]
    vec4 highlightTint; // rgb = Tint color applied to highlights, a = Strength [0,1]
    float contrast;     // 1.0 = Neutral, >1.0 = Punchier S-curve around mid-gray
    float saturation;   // 1.0 = Neutral, 0.0 = Grayscale, >1.0 = Oversaturated
} pc;

const uint kLutSize = 32U;

const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);
const float kContrastPivot = 0.435; // Roughly middle gray in gamma space (matches ~18% linear gray)

vec3 applyContrast(vec3 color, float contrast)
{
    return (color - vec3(kContrastPivot)) * contrast + vec3(kContrastPivot);
}

vec3 applySaturation(vec3 color, float saturation)
{
    float luma = dot(color, kLumaRec709);
    return mix(vec3(luma), color, saturation);
}

// Tints shadows and highlights independently based on a luminance mask, the classic "split toning" building block
// behind most cinematic film looks (e.g. teal & orange).
vec3 applySplitToning(vec3 color, vec4 shadowTint, vec4 highlightTint)
{
    float luma = dot(color, kLumaRec709);

    float shadowMask = 1.0 - smoothstep(0.0, 0.5, luma);
    float highlightMask = smoothstep(0.5, 1.0, luma);

    color = mix(color, color * shadowTint.rgb, shadowMask * shadowTint.a);
    color = mix(color, color * highlightTint.rgb, highlightMask * highlightTint.a);

    return color;
}

vec3 applyFilmLook(vec3 color)
{
    color = applyContrast(color, pc.contrast);
    color = applySaturation(color, pc.saturation);
    color = applySplitToning(color, pc.shadowTint, pc.highlightTint);
    return clamp(color, 0.0, 1.0);
}

void main()
{
    if (any(greaterThanEqual(gl_GlobalInvocationID, uvec3(kLutSize)))) {
        return;
    }

    // LUT domain axis = gamma-encoded [0,1] pixel value
    vec3 gammaColor = (vec3(gl_GlobalInvocationID) + 0.5) / float(kLutSize);
    vec3 graded = applyFilmLook(gammaColor);

    imageStore(uColorGradingLut, ivec3(gl_GlobalInvocationID), vec4(graded, 1.0));
}
