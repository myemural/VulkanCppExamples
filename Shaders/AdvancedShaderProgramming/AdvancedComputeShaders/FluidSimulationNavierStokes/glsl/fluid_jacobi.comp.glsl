#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 8, local_size_y = 8, local_size_z = 4) in;

layout(binding = 0, rgba16f) uniform readonly image3D field; // xyz = Advected Velocity
layout(binding = 1, r32f) uniform readonly image3D pressureSrc;
layout(binding = 2, r32f) uniform writeonly image3D pressureDst;

layout(push_constant) uniform FluidPushConstants {
    vec4 params;    // x = Time Step, y = Buoyancy, z = Dissipation, w = Cell Size
    uvec4 gridInfo; // xyz = Grid Resolution, w = 1 when the field must be seeded
} pc;

// Solid walls, the velocity outside the grid is zero
vec3 loadVelocity(ivec3 coord, ivec3 size)
{
    if (any(lessThan(coord, ivec3(0))) || any(greaterThanEqual(coord, size))) {
        return vec3(0.0);
    }

    return imageLoad(field, coord).xyz;
}

// Neumann boundary, a wall mirrors the pressure of the cell next to it
float loadPressure(ivec3 coord, ivec3 size, float center)
{
    if (any(lessThan(coord, ivec3(0))) || any(greaterThanEqual(coord, size))) {
        return center;
    }
    return imageLoad(pressureSrc, coord).r;
}

void main()
{
    ivec3 coord = ivec3(gl_GlobalInvocationID);
    ivec3 size = ivec3(pc.gridInfo.xyz);

    if (any(greaterThanEqual(coord, size))) {
        return;
    }

    float cellSize = pc.params.w;

    // The velocity field doesn't change while the solver iterates
    // So the recomputing it divergence here costs six loads and saves a whole extra pass and volume
    vec3 right = loadVelocity(coord + ivec3(1, 0, 0), size);
    vec3 left = loadVelocity(coord - ivec3(1, 0, 0), size);
    vec3 up = loadVelocity(coord + ivec3(0, 1, 0), size);
    vec3 down = loadVelocity(coord - ivec3(0, 1, 0), size);
    vec3 front = loadVelocity(coord + ivec3(0, 0, 1), size);
    vec3 back = loadVelocity(coord - ivec3(0, 0, 1), size);

    float divergence = 0.5 * ((right.x - left.x) + (up.y - down.y) + (front.z - back.z)) / cellSize;

    float center = imageLoad(pressureSrc, coord).r;
    float neighbourSum = loadPressure(coord + ivec3(1, 0, 0), size, center) +
                         loadPressure(coord - ivec3(1, 0, 0), size, center) +
                         loadPressure(coord + ivec3(0, 1, 0), size, center) +
                         loadPressure(coord - ivec3(0, 1, 0), size, center) +
                         loadPressure(coord + ivec3(0, 0, 1), size, center) +
                         loadPressure(coord - ivec3(0, 0, 1), size, center);

    // Discrete laplacian(p) = divergence, solved for the center cell
    imageStore(pressureDst, coord, vec4((neighbourSum - divergence * cellSize * cellSize) / 6.0));
}
