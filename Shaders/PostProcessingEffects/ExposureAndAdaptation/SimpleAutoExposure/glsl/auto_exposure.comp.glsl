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

layout(local_size_x = THREAD_COUNT_X, local_size_y = THREAD_COUNT_Y, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform sampler2D gLightPassOutput;

layout(std430, set = 0, binding = 1) writeonly buffer AutoExposureBuffer{
    float averageLuminance;
    float exposureEv;
} outExposure;

layout(push_constant) uniform AutoExposurePushConstants {
    float keyValue;        // Target middle-gray
    float minExposureEv;   // Lower EV clamp
    float maxExposureEv;   // Upper EV clamp
    uint sampleStep;       // Pixel stride (1 = Every Pixel)
} pc;

const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);
const float kEpsilon = 1e-4; // Prevents log(0) for black pixels

shared float sLogLuminanceSum[THREAD_COUNT];
shared uint sSampleCount[THREAD_COUNT];

void main()
{
    const ivec2 imageSize = textureSize(gLightPassOutput, 0);
    const int step = int(max(pc.sampleStep, 1U));
    const uint threadIndex = gl_LocalInvocationIndex;

    // Each thread accumulates a strided subset of the image
    float logLuminanceSum = 0.0;
    uint sampleCount = 0U;
    for (int y = int(gl_LocalInvocationID.y) * step; y < imageSize.y; y += THREAD_COUNT_Y * step) {
        for (int x = int(gl_LocalInvocationID.x) * step; x < imageSize.x; x += THREAD_COUNT_X * step) {
            const vec3 hdrColor = texelFetch(gLightPassOutput, ivec2(x, y), 0).rgb;
            const float luminance = dot(hdrColor, kLumaRec709);
            logLuminanceSum += log(max(luminance, kEpsilon));
            sampleCount++;
        }
    }

    sLogLuminanceSum[threadIndex] = logLuminanceSum;
    sSampleCount[threadIndex] = sampleCount;
    barrier();

    // Parallel reduction in shared memory
    for (uint stride = THREAD_COUNT / 2U; stride > 0U; stride >>= 1U) {
        if (threadIndex < stride) {
            sLogLuminanceSum[threadIndex] += sLogLuminanceSum[threadIndex + stride];
            sSampleCount[threadIndex] += sSampleCount[threadIndex + stride];
        }
        barrier();
    }

    if (threadIndex == 0U) {
        // Log-average (geometric mean) luminance
        const float averageLogLuminance = sLogLuminanceSum[0] / float(max(sSampleCount[0], 1U));
        const float averageLuminance = exp(averageLogLuminance);

        // Reinhard: scale = key / Lavg => EV = log2(key / Lavg)
        const float exposureEv = clamp(log2(pc.keyValue / averageLuminance), pc.minExposureEv, pc.maxExposureEv);

        outExposure.averageLuminance = averageLuminance;
        outExposure.exposureEv = exposureEv;
    }
}
