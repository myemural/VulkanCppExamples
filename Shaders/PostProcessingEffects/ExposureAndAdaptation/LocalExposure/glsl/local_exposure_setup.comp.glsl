#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define THREAD_COUNT_X 16
#define THREAD_COUNT_Y 16
#define THREAD_COUNT (THREAD_COUNT_X * THREAD_COUNT_Y)
#define DOWNSAMPLE_FACTOR 32 // Grid cell size in pixels; each thread handles a 2x2 pixel block
#define GRID_DEPTH 16        // Number of log2 luminance bins in the bilateral grid

layout(local_size_x = THREAD_COUNT_X, local_size_y = THREAD_COUNT_Y, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform sampler2D gSourceImage;
layout(set = 0, binding = 1, r32f) uniform writeonly image2D gLogLuminanceImage;
layout(set = 0, binding = 2, rgba16f) uniform writeonly image3D gBilateralGridImage;

layout(push_constant) uniform LocalExposureSetupPushConstants {
    float minLogLuminance;
    float logLuminanceRange;
} pc;

const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);
const float kEpsilon = 1e-8;
const float kFixedPointScale = 65536.0;

shared vec3 sColorSum[THREAD_COUNT];
shared uint sSampleCount[THREAD_COUNT];
shared uint sBinLogSum[GRID_DEPTH];
shared uint sBinCount[GRID_DEPTH];

void main()
{
    const uint threadIndex = gl_LocalInvocationIndex;
    const ivec2 sourceSize = textureSize(gSourceImage, 0);

    if (threadIndex < GRID_DEPTH) {
        sBinLogSum[threadIndex] = 0U;
        sBinCount[threadIndex] = 0U;
    }
    barrier();

    const ivec2 baseCoord = ivec2(gl_WorkGroupID.xy) * DOWNSAMPLE_FACTOR + ivec2(gl_LocalInvocationID.xy) * 2;

    vec3 colorSum = vec3(0.0);
    uint sampleCount = 0;
    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 2; ++x) {
            const ivec2 coord = baseCoord + ivec2(x, y);
            if (all(lessThan(coord, sourceSize))) {
                const vec3 color = texelFetch(gSourceImage, coord, 0).rgb;
                colorSum += color;
                ++sampleCount;

                // Splat into the bilateral grid (range axis = normalized log2 luminance)
                const float logLuminance = log2(max(dot(color, kLumaRec709), kEpsilon));
                const float normalized = clamp((logLuminance - pc.minLogLuminance) / pc.logLuminanceRange, 0.0, 1.0);
                const uint bin = min(uint(normalized * float(GRID_DEPTH)), uint(GRID_DEPTH - 1));
                atomicAdd(sBinLogSum[bin], uint(normalized * kFixedPointScale));
                atomicAdd(sBinCount[bin], 1U);
            }
        }
    }

    sColorSum[threadIndex] = colorSum;
    sSampleCount[threadIndex] = sampleCount;
    barrier();

    // Parallel reduction (mean color of the block)
    for (uint stride = THREAD_COUNT / 2U; stride > 0U; stride >>= 1U) {
        if (threadIndex < stride) {
            sColorSum[threadIndex] += sColorSum[threadIndex + stride];
            sSampleCount[threadIndex] += sSampleCount[threadIndex + stride];
        }
        barrier();
    }

    const ivec2 cellCoord = ivec2(gl_WorkGroupID.xy);

    // Blurred luminance input: Log of the mean color of the block
    if (threadIndex == 0U) {
        const vec3 meanColor = sColorSum[0] / float(max(sSampleCount[0], 1U));
        const float logLuminance = log2(max(dot(meanColor, kLumaRec709), kEpsilon));
        imageStore(gLogLuminanceImage, cellCoord, vec4(logLuminance, 0.0, 0.0, 0.0));
    }

    // Bilateral grid: Homogeneous (weighted mean log luminance, weight) per bin
    if (threadIndex < GRID_DEPTH) {
        const uint count = sBinCount[threadIndex];
        const float weight = float(count) / float(DOWNSAMPLE_FACTOR * DOWNSAMPLE_FACTOR);
        float meanLogLuminance = 0.0;
        if (count > 0U) {
            const float meanNormalized = (float(sBinLogSum[threadIndex]) / kFixedPointScale) / float(count);
            meanLogLuminance = pc.minLogLuminance + meanNormalized * pc.logLuminanceRange;
        }
        imageStore(gBilateralGridImage, ivec3(cellCoord, int(threadIndex)), vec4(meanLogLuminance * weight, weight, 0.0, 0.0));
    }
}
