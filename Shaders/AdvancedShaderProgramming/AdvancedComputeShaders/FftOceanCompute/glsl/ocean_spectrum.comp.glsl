#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define TWO_PI 6.28318530718

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(std140, binding = 0) uniform OceanUbo {
    vec4 patchParams; // x = Patch Length, y = Gravity, z = Time, w = Choppiness
    vec4 waveParams;  // x = Amplitude Scale, y = Foam Threshold, z = Foam Sharpness
    uvec4 gridInfo;   // x = FFT Size
} ocean;

layout(std430, binding = 1) readonly buffer InitialSpectrumBuffer { vec4 initialSpectrum[]; };
layout(std430, binding = 2) writeonly buffer SpectrumBuffer { vec4 spectrum[]; };

vec2 complexMul(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

void main()
{
    const uint size = ocean.gridInfo.x;
    const uvec2 coord = gl_GlobalInvocationID.xy;
    if (coord.x >= size || coord.y >= size) {
        return;
    }

    const uint index = coord.y * size + coord.x;

    // Wave vector, the spectrum is strored shifted, so index 0 maps to -N/2
    const vec2 waveIndex = vec2(coord) - float(size) * 0.5;
    const vec2 waveVector = TWO_PI * waveIndex / ocean.patchParams.x;
    const float waveLength = length(waveVector);

    if (waveLength < 1.0e-4) {
        spectrum[index] = vec4(0.0);
        return;
    }

    // Dispersion relation for deep water
    const float omega = sqrt(ocean.patchParams.y * waveLength);
    const float phase = omega * ocean.patchParams.z;
    const vec2 positive = vec2(cos(phase), sin(phase));
    const vec2 negative = vec2(cos(phase), -sin(phase));

    // h(k, t) = h0(k) * e^(i * w * t) + conj(h0(-k)) * e^(-i * w * t)
    const vec4 h0 = initialSpectrum[index];
    vec2 height = complexMul(h0.xy, positive) + complexMul(h0.zw, negative);
    height *= ocean.waveParams.x;

    // Horizontal (choppy) displacement
    const vec2 factor = vec2(waveVector.y, -waveVector.x) / waveLength;
    const vec2 displacement = complexMul(height, factor) * ocean.patchParams.w;

    spectrum[index] = vec4(height, displacement);
}
