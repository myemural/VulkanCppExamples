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
    vec2 texelStep = pc.direction * pc.srcTexelSize;
    int radius = int(pc.parameter);

    vec3 result = vec3(0.0);
    for (int i = -radius; i <= radius; ++i) {
        result += texture(uSourceImage, uv + texelStep * float(i)).rgb;
    }

    // Box blur has hard kernel edges, so bright objects get visible blocky/streaky borders
    outColor = vec4(result / float(2 * radius + 1), 1.0);
}
