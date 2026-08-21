#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 256) in;

const uint kTileSize = 256;

struct Particle
{
    vec4 positionMass; // xyz = Position, w = Mass
    vec4 velocityType; // xyz = Velocity, w = Galaxy Index (0.0 or 1.0)
};

layout(std430, binding = 0) readonly buffer PrevParticles {
    Particle prevParticles[];
};

layout(std430, binding = 1) writeonly buffer NextParticles {
    Particle nextParticles[];
};

layout (push_constant) uniform PushConstants {
    float deltaTime;
    float gravity;   // Gravitational constant (G)
    float softening; // Plummer softening length, prevents singularities
    float damping;   // Slight velocity damping, keeps the system bounded
    uint particleCount;
} pc;

// Staging area for one tile of bodies (xyz = Position, w = Mass)
shared vec4 sharedBodies[kTileSize];

void main()
{
    uint index = gl_GlobalInvocationID.x;
    uint localIndex = gl_LocalInvocationID.x;

    const uint safeIndex = min(index, pc.particleCount - 1U);

    const Particle self = prevParticles[safeIndex];
    const vec3 position = self.positionMass.xyz;

    vec3 acceleration = vec3(0.0);

    const uint tileCount = (pc.particleCount + kTileSize - 1U) / kTileSize;
    const float softeningSqr = pc.softening * pc.softening;

    for (uint tile = 0U; tile < tileCount; ++tile) {
        const uint loadIndex = tile * kTileSize + localIndex;

        // Padding entries get zero mass, so they contribute no force
        sharedBodies[localIndex] = (loadIndex < pc.particleCount) ? prevParticles[loadIndex].positionMass : vec4(0.0);

        barrier();

        for (uint j = 0U; j < kTileSize; ++j) {
            const vec3 direction = sharedBodies[j].xyz - position;

            // Softened Newtonian Gravity: a += G * m * d / (|d|^2 + eps^2)^(3/3)
            const float distanceSqr = dot(direction, direction) + softeningSqr;
            const float invDistance = 1.0 / sqrt(distanceSqr);
            const float invDistanceCubed = invDistance * invDistance * invDistance;

            acceleration += direction * (pc.gravity * sharedBodies[j].w * invDistanceCubed);
        }

        barrier();
    }

    if (index >= pc.particleCount) {
        return;
    }

    // Semi-implicit Euler integration
    vec3 velocity = (self.velocityType.xyz + acceleration * pc.deltaTime) * pc.damping;
    const vec3 nextPosition = position + velocity * pc.deltaTime;

    nextParticles[index].positionMass = vec4(nextPosition, self.positionMass.w);
    nextParticles[index].velocityType = vec4(velocity, self.velocityType.w);
}
