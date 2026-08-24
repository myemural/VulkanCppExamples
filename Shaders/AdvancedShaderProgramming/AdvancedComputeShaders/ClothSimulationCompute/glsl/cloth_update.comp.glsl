#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

struct ClothParticle
{
    vec4 position;     // xyz = Current Position, w = Inverse Mass (0 = Pinned)
    vec4 prevPosition; // xyz = Position at the beginning of the sub-step
    vec4 velocity;     // xyz = Velocity
    vec4 normal;       // xyz = Smooth Normal
};

struct ClothVertex
{
    vec4 position; // xyz = World-Space Position
    vec4 normal;   // xyz = World-Space Normal
};

layout(std140, binding = 0) uniform ClothUbo {
    vec4 gravityDeltaTime;   // xyz = Gravity, w = Sub-step Delta Time
    vec4 sphereCenterRadius; // xyz = Sphere Center, w = Sphere Radius
    vec4 params;             // x = Rest Distance, y = Damping, z = Friction, w = Ground Level
    vec4 stiffness;          // x = Thickness, y = Structural, z = Shear, w = Bend
    uvec4 gridInfo;          // x = Vertices per Side, y = Particle Count
} cloth;

layout(std430, binding = 1) readonly buffer ParticlesIn { ClothParticle particlesIn[]; };
layout(std430, binding = 2) writeonly buffer ParticlesOut { ClothParticle particlesOut[]; };
layout(std430, binding = 3) writeonly buffer ClothVertexBuffer { ClothVertex clothVertices[]; };

vec3 fetchPosition(ivec2 coord)
{
    const int side = int(cloth.gridInfo.x);
    const ivec2 clamped = clamp(coord, ivec2(0), ivec2(side - 1));
    return particlesIn[uint(clamped.y * side + clamped.x)].position.xyz;
}

void main()
{
    const uint side = cloth.gridInfo.x;
    const uvec2 coord = gl_GlobalInvocationID.xy;
    if (coord.x >= side || coord.y >= side) {
        return;
    }

    const uint index = coord.y * side + coord.x;
    ClothParticle particle = particlesIn[index];

    const float invMass = particle.position.w;
    const float dt = cloth.gravityDeltaTime.w;

    if (invMass > 0.0) {
        vec3 velocity = (particle.position.xyz - particle.prevPosition.xyz) / dt;

        // A particle must not travel more than half a cell per sub-step, otherwise it can tunnel
        const float maxSpeed = (cloth.params.x * 0.5) / dt;
        const float speed = length(velocity);
        if (speed > maxSpeed) {
            velocity *= maxSpeed / speed;
        }

        particle.velocity = vec4(velocity, 0.0);
    }

    // Smooth normal from central diffrerences over the particle grid
    const ivec2 selfCoord = ivec2(coord);
    const vec3 tangentX = fetchPosition(selfCoord + ivec2(1, 0)) - fetchPosition(selfCoord - ivec2(1, 0));
    const vec3 tangentZ = fetchPosition(selfCoord + ivec2(0, 1)) - fetchPosition(selfCoord - ivec2(0, 1));

    vec3 normal = cross(tangentZ, tangentX);
    const float normalLength = length(normal);
    normal = (normalLength > 1e-6) ? (normal / normalLength) : vec3(0.0, 1.0, 0.0);
    particle.normal = vec4(normal, 0.0);

    particlesOut[index] = particle;

    clothVertices[index].position = vec4(particle.position.xyz, 1.0);
    clothVertices[index].normal = vec4(normal, 0.0);
}
