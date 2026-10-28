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

layout(push_constant) uniform BlurPushConstants {
    vec2 texelStep; // (1 / width, 0) horizontal pass, (0, 1 / height) vertical pass
    float sigma;    // Standard deviation of the gaussian kernel (in texels)
} pc;

// Upper limit for the kernel radius (in texels), "3 * sigma" covers ~99.7% of the gaussian
const int kMaxKernelRadius = 16;

float gaussianWeight(float x, float sigma)
{
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

void main()
{
    // Source and target images have the same (half) resolution
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(uSourceImage, 0));

    float sigma = max(pc.sigma, 0.5);
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

        vec2 uvOffset = pc.texelStep * offset;
        result += texture(uSourceImage, uv + uvOffset).rgb * weight;
        result += texture(uSourceImage, uv - uvOffset).rgb * weight;
        weightSum += 2.0 * weight;
    }

    // Normalization keeps the total energy of the bloom independent from sigma
    outColor = vec4(result / weightSum, 1.0);
}
