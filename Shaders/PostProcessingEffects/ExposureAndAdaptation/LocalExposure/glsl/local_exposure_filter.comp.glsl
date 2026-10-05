#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define THREAD_COUNT_X 8
#define THREAD_COUNT_Y 8

layout(local_size_x = THREAD_COUNT_X, local_size_y = THREAD_COUNT_Y, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform sampler2D gLogLuminanceImage;
layout(set = 0, binding = 1) uniform sampler3D gBilateralGridImage;
layout(set = 0, binding = 2, r32f) uniform writeonly image2D gBlurredLogLuminanceImage;
layout(set = 0, binding = 3, rgba16f) uniform writeonly image3D gBlurredBilateralGridImage;

layout(push_constant) uniform LocalExposureFilterPushConstants {
    int blurRadius; // In low-resolution texels
    float blurSigma;
} pc;

const float kBinomialWeights[3] = float[](1.0, 2.0, 1.0); // Separable [1 2 1] kernel

void main()
{
    const ivec3 gridSize = textureSize(gBilateralGridImage, 0);
    const ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(coord, gridSize.xy))) {
        return;
    }

    // Blurred luminance calculation
    const float inverseTwoSigmaSquared = 1.0 / (2.0 * pc.blurSigma * pc.blurSigma);
    float weightedSum = 0.0;
    float weightSum = 0.0;
    for (int y = -pc.blurRadius; y <= pc.blurRadius; ++y) {
        for (int x = -pc.blurRadius; x <= pc.blurRadius; ++x) {
            const ivec2 sampleCoord = clamp(coord + ivec2(x, y), ivec2(0), gridSize.xy - 1);
            const float weight = exp(-float(x * x + y * y) * inverseTwoSigmaSquared);
            weightedSum += weight * texelFetch(gLogLuminanceImage, sampleCoord, 0).r;
            weightSum += weight;
        }
    }
    imageStore(gBlurredLogLuminanceImage, coord, vec4(weightedSum / max(weightSum, 1e-8), 0.0, 0.0, 0.0));

    // Blurred bilatreal grid calculation with 3x3x3 binomial blur in (x, y, luminance)
    for (int z = 0; z < gridSize.z; ++z) {
        vec2 accumulated = vec2(0.0);
        for (int dz = -1; dz <= 1; ++dz) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    const ivec3 sampleCoord = clamp(ivec3(coord, z) + ivec3(dx, dy, dz), ivec3(0), gridSize - 1);
                    const float weight = kBinomialWeights[dx + 1] * kBinomialWeights[dy + 1] * kBinomialWeights[dz + 1];
                    accumulated += weight * texelFetch(gBilateralGridImage, sampleCoord, 0).rg;
                }
            }
        }
        imageStore(gBlurredBilateralGridImage, ivec3(coord, z), vec4(accumulated / 64.0, 0.0, 0.0));
    }
}
