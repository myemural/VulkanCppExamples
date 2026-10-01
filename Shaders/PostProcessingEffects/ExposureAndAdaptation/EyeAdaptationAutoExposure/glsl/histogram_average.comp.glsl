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

layout(std430, set = 0, binding = 2) buffer AutoExposureBuffer{
    float averageLuminance;    // Adapted luminance (linear)
    float exposureEv;          // Read by the tone mapping pass
    float targetLogLuminance;  // This frame's metered value
    float adaptedLogLuminance; // Persistent adaptation state
} exposureData;

layout(push_constant) uniform AutoExposurePushConstants {
    float minLogLuminance;
    float logLuminanceRange;
    float lowPercentile;
    float highPercentile;
    float keyValue;
    float minExposureEv;
    float maxExposureEv;
    float deltaTime;
    float darkToBrightAdaptationSpeed;
    float brightToDarkAdaptationSpeed;
    uint resetAdaptation; // For the reset of the first frame
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
        // Target (metered) log luminance of this frame
        float targetLogLuminance = sWeightedLogSum[0] / sWeightSum[0];

        // Temporal (eye) adaptation to log2 space
        float adaptedLogLuminance;
        if (pc.resetAdaptation != 0U) {
            adaptedLogLuminance = targetLogLuminance; // Buffer content is undefined on the first frame
        } else {
            const float prevLogLuminance = exposureData.adaptedLogLuminance;
            const float speed = (targetLogLuminance > prevLogLuminance) ? pc.darkToBrightAdaptationSpeed
                                                                        : pc.brightToDarkAdaptationSpeed;

            // Frame-rate independent exponential smoothing
            const float blend = 1.0 - exp(-pc.deltaTime * speed);
            adaptedLogLuminance = prevLogLuminance + (targetLogLuminance - prevLogLuminance) * blend;
        }

        exposureData.targetLogLuminance = targetLogLuminance;
        exposureData.adaptedLogLuminance = adaptedLogLuminance;
        exposureData.averageLuminance = exp2(adaptedLogLuminance);
        exposureData.exposureEv = clamp(log2(pc.keyValue) - adaptedLogLuminance, pc.minExposureEv, pc.maxExposureEv);;
    }
}
