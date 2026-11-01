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

// Upper limit for the kernel radius (in texels), "3 * sigma" covers ~99.7% of the gaussian
const int kMaxKernelRadius = 16;

float gaussianWeight(float x, float sigma)
{
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

void main()
{
    vec2 uv = gl_FragCoord.xy * pc.dstTexelSize;
    vec2 texelStep = pc.direction * pc.srcTexelSize;

    float sigma = max(pc.parameter, 0.5);
    int radius = min(int(ceil(3.0 * sigma)), kMaxKernelRadius);

    // Center tap
    float weightSum = gaussianWeight(0.0, sigma);
    vec3 result = texture(uSourceImage, uv).rgb * weightSum;

    // Taps "i" and "i + 1" are fetched together
    for (int i = 1; i <= radius; i += 2) {
        float weightA = gaussianWeight(float(i), sigma);
        float weightB = (i + 1 <= radius) ? gaussianWeight(float(i + 1), sigma) : 0.0;
        float weight = weightA + weightB;
        float offset = (float(i) * weightA + float(i + 1) * weightB) / weight;

        vec2 uvOffset = texelStep * offset;
        result += texture(uSourceImage, uv + uvOffset).rgb * weight;
        result += texture(uSourceImage, uv - uvOffset).rgb * weight;
        weightSum += 2.0 * weight;
    }

    outColor = vec4(result / weightSum, 1.0);
}
