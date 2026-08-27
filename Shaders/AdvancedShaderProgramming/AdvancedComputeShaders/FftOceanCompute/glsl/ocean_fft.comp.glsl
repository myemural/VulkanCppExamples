#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define TWO_PI 6.28318530718

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 2) buffer SpectrumBuffer { vec4 spectrum[]; };

layout(push_constant) uniform FftPushConstants {
    uint direction; // 0 = Rows (horizontal), 1 = Columns (vertical)
} pc;

const uint kFftSize = 256;
const uint kFftStages = 8;
const uint kThreadCount = kFftSize / 2U;

shared vec4 sharedData[kFftSize];

vec2 complexMul(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

uint bufferIndex(uint pos)
{
    const uint line = gl_WorkGroupID.x;
    return (pc.direction == 0U) ? (line * kFftSize + pos) : (pos * kFftSize + line);
}

uint reverseIndex(uint value)
{
    return bitfieldReverse(value) >> (32U - kFftStages);
}

void main()
{
    const uint thread = gl_LocalInvocationID.x;

    // Load the line into shared memory in bit-reversed order
    sharedData[reverseIndex(thread)] = spectrum[bufferIndex(thread)];
    sharedData[reverseIndex(thread + kThreadCount)] = spectrum[bufferIndex(thread + kThreadCount)];
    barrier();

    for (uint stage = 0U; stage < kFftStages; ++stage) {
        const uint halfSize = 1U << stage;
        const uint blockSize = halfSize << 1U;

        // Butterfly pair owned by this thread
        const uint offset = thread & (halfSize - 1U);
        const uint lower = (thread / halfSize) * blockSize + offset;
        const uint upper = lower + halfSize;

        // Positive exponent, this is a inverse transform, left unnormalized
        const float angle = TWO_PI * float(offset) / float(blockSize);
        const vec2 twiddle = vec2(cos(angle), sin(angle));

        const vec4 lowerVal = sharedData[lower];
        const vec4 upperVal = sharedData[upper];
        const vec4 rotated = vec4(complexMul(upperVal.xy, twiddle), complexMul(upperVal.zw, twiddle));

        sharedData[lower] = lowerVal + rotated;
        sharedData[upper] = lowerVal - rotated;
        barrier();
    }

    spectrum[bufferIndex(thread)] = sharedData[thread];
    spectrum[bufferIndex(thread + kThreadCount)] = sharedData[thread + kThreadCount];
}
