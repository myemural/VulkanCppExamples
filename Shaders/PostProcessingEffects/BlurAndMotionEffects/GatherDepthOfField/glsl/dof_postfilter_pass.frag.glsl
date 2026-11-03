#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D uSourceImage;

layout(push_constant) uniform BloomPassPushConstants {
    vec2 srcTexelSize;   // 1.0 / source image size
    vec2 dstTexelSize;   // 1.0 / target mip size
    float focusDistance; // Focus distance in meters (view-space units)
    float cocScale;      // CoC radius (in half-res pixels) of an object at infinity
    float maxCocRadius;  // Maximum CoC radius in half-res pixels
} pc;

void main()
{
    vec2 uv = gl_FragCoord.xy * pc.dstTexelSize;
    vec2 t = pc.srcTexelSize * 0.5;

    vec4 result = texture(uSourceImage, uv + vec2(-t.x, -t.y));
    result += texture(uSourceImage, uv + vec2(t.x, -t.y));
    result += texture(uSourceImage, uv + vec2(-t.x, t.y));
    result += texture(uSourceImage, uv + vec2(t.x, t.y));

    outColor = result * 0.25;
}
