#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0) uniform sampler2D inputShadowMap;

layout(binding = 1, rgba32f) writeonly uniform image2D outputShadowMap;

// Gaussian weights (5x5 kernel)
const float kernel[5][5] = float[5][5](
    float[5](1,  4,  6,  4, 1),
    float[5](4, 16, 24, 16, 4),
    float[5](6, 24, 36, 24, 6),
    float[5](4, 16, 24, 16, 4),
    float[5](1,  4,  6,  4, 1)
);

const float kernelSum = 256.0;

void main()
{
    ivec2 imgSize = imageSize(outputShadowMap);
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);

    // Bounds check
    if (coord.x >= imgSize.x || coord.y >= imgSize.y) {
        return;
    }

    // 5x5 convolution
    vec4 result = vec4(0.0);
    for (int y = -2; y <= 2; ++y) {
        for (int x = -2; x <= 2; ++x) {
            ivec2 sampleCoord = coord + ivec2(x, y);
            sampleCoord = clamp(sampleCoord, ivec2(0), imgSize - 1);

            vec4 sampleValue = texelFetch(inputShadowMap, sampleCoord, 0);
            float weight = kernel[y + 2][x + 2];

            result += sampleValue * weight;
        }
    }

    result /= kernelSum;

    imageStore(outputShadowMap, coord, result);
}