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
#define BIN_COUNT 256

layout(local_size_x = THREAD_COUNT_X, local_size_y = THREAD_COUNT_Y, local_size_z = 1) in;

layout(set = 0, binding = 0) uniform sampler2D gLightPassOutput;

layout(std430, set = 0, binding = 1) buffer HistogramBuffer{
    uint bins[BIN_COUNT];
} histogram;

layout(push_constant) uniform AutoExposurePushConstants {
    float minLogLuminance;
    float logLuminanceRange;
    float lowPercentile;
    float highPercentile;
    float keyValue;
    float minExposureEv;
    float maxExposureEv;
} pc;

const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);
const float kEpsilon = 1e-8;

shared uint sBins[BIN_COUNT];

uint luminanceToBin(float luminance)
{
    const float logLuminance = log2(max(luminance, kEpsilon));
    const float normalized = clamp((logLuminance - pc.minLogLuminance) / pc.logLuminanceRange, 0.0, 1.0);
    return min(uint(normalized * float(BIN_COUNT)), uint(BIN_COUNT - 1));
}

void main()
{
    const uint threadIndex = gl_LocalInvocationIndex;
    sBins[threadIndex] = 0U;
    barrier();

    const ivec2 imageSize = textureSize(gLightPassOutput, 0);
    const ivec2 coord = ivec2(gl_GlobalInvocationID.xy);

    if (all(lessThan(coord, imageSize))) {
        const vec3 hdrSample = texelFetch(gLightPassOutput, coord, 0).rgb;
        atomicAdd(sBins[luminanceToBin(dot(hdrSample, kLumaRec709))], 1U);
    }
    barrier();

    // Merge the local histogram into the global one
    if (sBins[threadIndex] > 0U) {
        atomicAdd(histogram.bins[threadIndex], sBins[threadIndex]);
    }
}
