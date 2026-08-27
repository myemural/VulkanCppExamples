#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

struct OceanVertex
{
    vec4 displacement; // xyz = World-Space Displacement, w = Foam Factor
    vec4 normal;       // xyz = World-Space Normal
};

layout(std140, binding = 0) uniform OceanUbo {
    vec4 patchParams; // x = Patch Length, y = Gravity, z = Time, w = Choppiness
    vec4 waveParams;  // x = Amplitude Scale, y = Foam Threshold, z = Foam Sharpness
    uvec4 gridInfo;   // x = FFT Size
} ocean;

layout(std430, binding = 2) readonly buffer SpectrumBuffer { vec4 spectrum[]; };
layout(std430, binding = 3) writeonly buffer OceanVertexBuffer { OceanVertex oceanVertices[]; };

// Fetches the displacement of a grid cell as (Dx, height, Dz)
vec3 fetchDisplacement(ivec2 coord)
{
    const int size = int(ocean.gridInfo.x);
    const ivec2 wrapped = ivec2(mod(vec2(coord), vec2(size)));
    const uint index = uint(wrapped.y * size + wrapped.x);

    // The spectrum was stored shifted by N/2, which corresponds to a (-1)^(x + z) modulation here
    const float shiftSign = ((wrapped.x + wrapped.y) % 2 == 0) ? 1.0 : -1.0;

    const vec4 value = spectrum[index];
    return vec3(value.z, value.x, value.w) * shiftSign;
}

void main()
{
    const uint size = ocean.gridInfo.x;
    const uvec2 coord = gl_GlobalInvocationID.xy;
    if (coord.x >= size || coord.y >= size) {
        return;
    }

    const ivec2 texel = ivec2(coord);
    const float spacing = ocean.patchParams.x / float(size); // World-space distance betwwen two vertices

    const vec3 center = fetchDisplacement(texel);
    const vec3 left = fetchDisplacement(texel + ivec2(-1, 0));
    const vec3 right = fetchDisplacement(texel + ivec2(1, 0));
    const vec3 back = fetchDisplacement(texel + ivec2(0, -1));
    const vec3 front = fetchDisplacement(texel + ivec2(0, 1));

    // Normal from the displaced neighbour positions
    const vec3 tangentX = (vec3(spacing, 0.0, 0.0) + right) - (vec3(-spacing, 0.0, 0.0) + left);
    const vec3 tangentZ = (vec3(0.0, 0.0, spacing) + front) - (vec3(0.0, 0.0, -spacing) + back);
    const vec3 normal = normalize(cross(tangentZ, tangentX));

    // Jacobian of the horizontal displacement
    const float invStep = 1.0 / (2.0 * spacing);
    const float dxdx = 1.0 + (right.x - left.x) * invStep;
    const float dzdz = 1.0 + (front.z - back.z) * invStep;
    const float dxdz = (front.x - back.x) * invStep;
    const float dzdx = (right.z - left.z) * invStep;
    const float jacobian = dxdx * dzdz - dxdz * dzdx;

    const float threshold = ocean.waveParams.y;
    const float foam = pow(clamp((threshold - jacobian) / max(threshold, 1.0e-4), 0.0, 1.0), ocean.waveParams.z);

    const uint index = coord.y * size + coord.x;
    oceanVertices[index].displacement = vec4(center, foam);
    oceanVertices[index].normal = vec4(normal, 0.0);
}
