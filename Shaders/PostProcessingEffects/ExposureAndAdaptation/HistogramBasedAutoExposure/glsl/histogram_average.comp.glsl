#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define BIN_COUNT 256

layout(local_size_x = BIN_COUNT, local_size_y = 1, local_size_z = 1) in;

layout(std430, set = 0, binding = 1) readonly buffer HistogramBuffer{
    uint bins[BIN_COUNT];
} histogram;

layout(std430, set = 0, binding = 2) writeonly buffer AutoExposureBuffer{
    float averageLuminance;
    float exposureEv;
} outExposure;

layout(push_constant) uniform AutoExposurePushConstants {
    float minLogLuminance;
    float logLuminanceRange;
    float lowPercentile;
    float highPercentile;
    float keyValue;
    float minExposureEv;
    float maxExposureEv;
} pc;

shared uint sPrefixSum[BIN_COUNT];
shared float sWeightedLogSum[BIN_COUNT];
shared float sWeightSum[BIN_COUNT];

void main()
{
    const uint binIndex = gl_LocalInvocationIndex;
    const uint binCount = histogram.bins[binIndex];

    // Inclusive prefix sum
    sPrefixSum[binIndex] = binCount;
    barrier();
    for (uint offset = 1U; offset < BIN_COUNT; offset <<= 1U) {
        const uint value = (binIndex >= offset) ? sPrefixSum[binIndex - offset] : 0U;
        barrier();
        sPrefixSum[binIndex] += value;
        barrier();
    }

    const float totalCount = float(sPrefixSum[BIN_COUNT - 1U]);
    const float lowCount = totalCount * pc.lowPercentile;
    const float highCount = totalCount * pc.highPercentile;

    // Pixel range covered by this bin: [binStart, binEnd)
    const float binEnd = float(sPrefixSum[binIndex]);
    const float binStart = binEnd - float(binCount);
    const float weight = max(min(binEnd, highCount) - max(binStart, lowCount), 0.0);

    // Log2 luminance at the center of the bin
    const float binLogLuminance = pc.minLogLuminance + ((float(binIndex) + 0.5) / float(BIN_COUNT)) * pc.logLuminanceRange;

    sWeightedLogSum[binIndex] = weight * binLogLuminance;
    sWeightSum[binIndex] = weight;
    barrier();

    // Parallel reduction in shared memory
    for (uint stride = BIN_COUNT / 2U; stride > 0U; stride >>= 1U) {
        if (binIndex < stride) {
            sWeightedLogSum[binIndex] += sWeightedLogSum[binIndex + stride];
            sWeightSum[binIndex] += sWeightSum[binIndex + stride];
        }
        barrier();
    }

    if (binIndex == 0U) {
        // Log-average (geometric mean) luminance
        const float averageLogLuminance = sWeightedLogSum[0] / sWeightSum[0];
        const float averageLuminance = exp(averageLogLuminance);
        const float exposureEv = clamp(log2(pc.keyValue) - averageLogLuminance, pc.minExposureEv, pc.maxExposureEv);

        outExposure.averageLuminance = averageLuminance;
        outExposure.exposureEv = exposureEv;
    }
}
