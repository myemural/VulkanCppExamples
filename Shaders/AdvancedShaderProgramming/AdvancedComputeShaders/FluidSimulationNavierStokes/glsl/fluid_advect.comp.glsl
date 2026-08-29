#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 8, local_size_y = 8, local_size_z = 4) in;

layout(binding = 0) uniform sampler3D fieldSrc; // xyz = Velocity, w = Smoke Density
layout(binding = 1, rgba16f) uniform writeonly image3D fieldDst;
layout(binding = 2, r32f) uniform writeonly image3D pressureDst;

layout(push_constant) uniform FluidPushConstants {
    vec4 params;    // x = Time Step, y = Buoyancy, z = Dissipation, w = Cell Size
    uvec4 gridInfo; // xyz = Grid Resolution, w = 1 when the field must be seeded
} pc;

// Initial blast, in normalized volume coordinates
const float kSeedHeight = 0.16;   // Height of the blob center above the floor
const float kSeedHalfSize = 0.09; // Half edge length
const float kSeedSpeed = 1.2;     // World units per second

void main()
{
    ivec3 coord = ivec3(gl_GlobalInvocationID);
    ivec3 size = ivec3(pc.gridInfo.xyz);

    if (any(greaterThanEqual(coord, size))) {
        return;
    }

    // The Jacobi solver always starts from a zero guess
    imageStore(pressureDst, coord, vec4(0.0));

    vec3 uvw = (vec3(coord) + 0.5) / vec3(size);

    // The outermost cell layer acts as a solid wall
    bool isWall = any(equal(coord, ivec3(0))) || any(equal(coord, size - ivec3(1)));

    if (pc.gridInfo.w == 1U) {
        // Reset: Drop a cube of smoke in and give it an upward kick
        vec3 widthRatio = float(size.x) / vec3(size);
        vec3 seedCenter = vec3(0.5, kSeedHeight * widthRatio.y, 0.5);

        bool inside = !isWall && all(lessThan(abs(uvw - seedCenter), kSeedHalfSize * widthRatio));
        float kick = kSeedSpeed * (1.0 + 0.15 * sin(uvw.x * 37.0) * sin(uvw.z * 41.0));

        imageStore(fieldDst, coord, inside ? vec4(0.0, kick, 0.0, 1.0) : vec4(0.0));
        return;
    }

    float timeStep = pc.params.x;
    float cellSize = pc.params.w;

    // Semi-Lagrangian advection: Follow the velocity backwards and pick up whatever was there
    // Velocity and smoke live in the same texel, so a single trilinear fetch advects both
    vec4 field = textureLod(fieldSrc, uvw, 0.0);
    vec3 sourceUvw = uvw - timeStep * field.xyz / (cellSize * vec3(size));
    vec4 advected = textureLod(fieldSrc, sourceUvw, 0.0);

    // Buoyancy: The smoke is hot, so it rises, the resulting shear is what rolls the blob into a vortex ring
    advected.y += pc.params.y * advected.w * timeStep;
    advected.w *= pc.params.z;

    // Solid walls stop the flow, but they must not eat the smoke
    if (isWall) {
        advected.xyz = vec3(0.0);
    }

    imageStore(fieldDst, coord, advected);
}
