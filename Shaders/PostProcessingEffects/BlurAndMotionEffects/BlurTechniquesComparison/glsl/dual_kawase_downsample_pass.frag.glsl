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
    vec2 srcTexelSize;  // 1.0 / source image size
    vec2 dstTexelSize;  // 1.0 / target mip size
    vec2 direction;     // (1, 0): Horizontal, (0, 1): Vertical (only for separable blurs)
    float parameter;    // Box: Radius, Gaussian: Sigma, Kawase: Sample offset, Dual Kawase: Sample offset
} pc;

void main()
{
    vec2 uv = gl_FragCoord.xy * pc.dstTexelSize;

    // The center of a target pixel lies on the corner of 4 source texels
    // Diagonal samples one source texel away average the neighbouring 2x2 blocks
    vec2 offset = pc.srcTexelSize * pc.parameter;

    vec3 result = texture(uSourceImage, uv).rgb * 4.0;
    result += texture(uSourceImage, uv + vec2(-offset.x, -offset.y)).rgb;
    result += texture(uSourceImage, uv + vec2(offset.x, -offset.y)).rgb;
    result += texture(uSourceImage, uv + vec2(-offset.x, offset.y)).rgb;
    result += texture(uSourceImage, uv + vec2(offset.x, offset.y)).rgb;

    outColor = vec4(result / 8.0, 1.0);
}
