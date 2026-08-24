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

// Over-relaxation of the averaged Jacobi correction, speeds up convergence
const float RELAXATION = 1.5;

// Neighbour offsets: 4 structural (edges), 4 shear (diagonals), 4 bend (2 cells away)
const ivec2 STRUCTURAL_OFFSETS[4] = ivec2[4](ivec2(1, 0), ivec2(-1, 0), ivec2(0, 1), ivec2(0, -1));
const ivec2 SHEAR_OFFSETS[4] = ivec2[4](ivec2(1, 1), ivec2(-1, 1), ivec2(1, -1), ivec2(-1, -1));
const ivec2 BEND_OFFSETS[4] = ivec2[4](ivec2(2, 0), ivec2(-2, 0), ivec2(0, 2), ivec2(0, -2));

vec3 correctionSum = vec3(0.0);
uint constraintCount = 0U;

// Accumulates a single distance constraint correction for this particle only (Jacobi)
void accumulateConstraint(vec3 position, float invMass, ivec2 neighbourCoord, float restLength, float stiffness)
{
    const int side = int(cloth.gridInfo.x);
    if (neighbourCoord.x < 0 || neighbourCoord.x >= side || neighbourCoord.y < 0 || neighbourCoord.y >= side) {
        return;
    }

    const uint neighbourIndex = uint(neighbourCoord.y * side + neighbourCoord.x);
    const vec4 neighbour = particlesIn[neighbourIndex].position;

    const vec3 delta = position - neighbour.xyz;
    const float distance = length(delta);
    const float invMassSum = invMass + neighbour.w;
    if (distance < 1e-6 || invMassSum <= 0.0) {
        return;
    }

    // Only this particle's share of the correction is applied, the neighbour fixes its own share
    const float scale = (invMass / invMassSum) * (distance - restLength) * stiffness;
    correctionSum -= (delta / distance) * scale;
    constraintCount++;
}

// Pushes the particle out of a collider and damps the tangential motion to fake friction
vec3 resolveCollision(vec3 position, vec3 prevPosition, vec3 normal, float penetartion, float friction)
{
    position += normal * penetartion;

    const vec3 movement = position - prevPosition;
    const vec3 tangential = movement - dot(movement, normal) * normal;

    return position - tangential * friction;
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
    vec3 position = particle.position.xyz;

    if (invMass > 0.0) {
        const float restDistance = cloth.params.x;
        const ivec2 selfCoord = ivec2(coord);

        for (int i = 0; i < 4; i++) {
            accumulateConstraint(position, invMass, selfCoord + STRUCTURAL_OFFSETS[i], restDistance, cloth.stiffness.y);
            accumulateConstraint(position, invMass, selfCoord + SHEAR_OFFSETS[i], restDistance * 1.41421356, cloth.stiffness.z);
            accumulateConstraint(position, invMass, selfCoord + BEND_OFFSETS[i], restDistance * 2.0, cloth.stiffness.w);
        }

        if (constraintCount > 0U) {
            position += (correctionSum / float(constraintCount)) * RELAXATION;
        }

        // Collision with sphere
        const vec3 sphereCenter = cloth.sphereCenterRadius.xyz;
        const float collisionRadius = cloth.sphereCenterRadius.w + cloth.stiffness.x;
        const vec3 toParticle = position - sphereCenter;
        const float centerDistance = length(toParticle);
        if (centerDistance < collisionRadius && centerDistance > 1e-6) {
            position = resolveCollision(position, particle.prevPosition.xyz, toParticle / centerDistance,
                                        collisionRadius - centerDistance, cloth.params.z);
        }

        // Collision with ground plane
        const float groundHeight = cloth.params.w + cloth.stiffness.x;
        if (position.y < groundHeight) {
            position = resolveCollision(position, particle.prevPosition.xyz, vec3(0.0, 1.0, 0.0),
                                        groundHeight - position.y, cloth.params.z);
        }
    }

    particle.position = vec4(position, invMass);
    particlesOut[index] = particle;
}
