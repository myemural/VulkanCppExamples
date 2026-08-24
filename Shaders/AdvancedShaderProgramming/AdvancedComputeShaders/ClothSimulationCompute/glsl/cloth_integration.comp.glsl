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

    vec3 position = particle.position.xyz;
    vec3 velocity = particle.velocity.xyz;

    // Previous position is the reference for the velocity update at the end pf the sub-step
    particle.prevPosition = vec4(position, 0.0);

    if (invMass > 0.0) {
        velocity += cloth.gravityDeltaTime.xyz * dt;
        velocity *= cloth.params.y; // Global damping, keeps the cloth from oscillating forever
        position += velocity * dt;
    } else {
        velocity = vec3(0.0);
    }

    particle.position = vec4(position, invMass);
    particle.velocity = vec4(velocity, 0.0);

    particlesOut[index] = particle;
}
