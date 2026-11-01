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

    // Half texel of the source image
    vec2 halfTexel = 0.5 * pc.srcTexelSize * pc.parameter;

    vec3 result = texture(uSourceImage, uv + vec2(-2.0 * halfTexel.x, 0.0)).rgb;
    result += texture(uSourceImage, uv + vec2(2.0 * halfTexel.x, 0.0)).rgb;
    result += texture(uSourceImage, uv + vec2(0.0, -2.0 * halfTexel.y)).rgb;
    result += texture(uSourceImage, uv + vec2(0.0, 2.0 * halfTexel.y)).rgb;
    result += texture(uSourceImage, uv + vec2(-halfTexel.x, -halfTexel.y)).rgb * 2.0;
    result += texture(uSourceImage, uv + vec2(halfTexel.x, -halfTexel.y)).rgb * 2.0;
    result += texture(uSourceImage, uv + vec2(-halfTexel.x, halfTexel.y)).rgb * 2.0;
    result += texture(uSourceImage, uv + vec2(halfTexel.x, halfTexel.y)).rgb * 2.0;

    outColor = vec4(result / 12.0, 1.0);
}
