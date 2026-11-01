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

    // Iteration index + 0.5: Every sample lies on the corner of 4 texels and averages them with bilinear filter
    vec2 offset = (pc.parameter + 0.5) * pc.srcTexelSize;

    vec3 result = texture(uSourceImage, uv + vec2(-offset.x, -offset.y)).rgb;
    result += texture(uSourceImage, uv + vec2(offset.x, -offset.y)).rgb;
    result += texture(uSourceImage, uv + vec2(-offset.x, offset.y)).rgb;
    result += texture(uSourceImage, uv + vec2(offset.x, offset.y)).rgb;

    outColor = vec4(result * 0.25, 1.0);
}
