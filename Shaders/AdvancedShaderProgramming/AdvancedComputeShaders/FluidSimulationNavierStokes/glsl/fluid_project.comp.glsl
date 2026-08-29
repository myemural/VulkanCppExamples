#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 8, local_size_y = 8, local_size_z = 4) in;

layout(binding = 0, rgba16f) uniform readonly image3D fieldSrc;
layout(binding = 1, r32f) uniform readonly image3D pressure;
layout(binding = 2, rgba16f) uniform writeonly image3D fieldDst;

layout(push_constant) uniform FluidPushConstants {
    vec4 params;    // x = Time Step, y = Buoyancy, z = Dissipation, w = Cell Size
    uvec4 gridInfo; // xyz = Grid Resolution, w = 1 when the field must be seeded
} pc;

float loadPressure(ivec3 coord, ivec3 size, float center)
{
    if (any(lessThan(coord, ivec3(0))) || any(greaterThanEqual(coord, size))) {
        return center;
    }
    return imageLoad(pressure, coord).r;
}

void main()
{
    ivec3 coord = ivec3(gl_GlobalInvocationID);
    ivec3 size = ivec3(pc.gridInfo.xyz);

    if (any(greaterThanEqual(coord, size))) {
        return;
    }

    vec4 field = imageLoad(fieldSrc, coord);

    // No-slip walls, but the smoke density still has to survive the copy
    if (any(equal(coord, ivec3(0))) || any(equal(coord, size - ivec3(1)))) {
        imageStore(fieldDst, coord, vec4(0.0, 0.0, 0.0, field.w));
        return;
    }

    float center = imageLoad(pressure, coord).r;
    vec3 gradient = 0.5 *
                    vec3(loadPressure(coord + ivec3(1, 0, 0), size, center) - loadPressure(coord - ivec3(1, 0, 0), size, center),
                         loadPressure(coord + ivec3(0, 1, 0), size, center) - loadPressure(coord - ivec3(0, 1, 0), size, center),
                         loadPressure(coord + ivec3(0, 0, 1), size, center) - loadPressure(coord - ivec3(0, 0, 1), size, center)) /
                    pc.params.w;

    imageStore(fieldDst, coord, vec4(field.xyz - gradient, field.w));
}
