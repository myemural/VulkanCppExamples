#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 256) in;

struct Particle {
    vec3 position;
    vec3 velocity;
    float lifetime;
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

layout(push_constant) uniform PushConstants {
    float deltaTime;
    int particleCount;
} pc;

float GetRandomNumber(uint seed)
{
    return fract(sin(float(seed) * 12.345) * 4567.89);
}

void main()
{
    uint id = gl_GlobalInvocationID.x;
    if (id >= pc.particleCount) {
        return;
    }

    Particle p = particles[id];

    if (p.lifetime <= 0.0 || isnan(p.lifetime)) {
        // Conical spawn
        float angle = GetRandomNumber(id) * 6.2831853;
        float radius = GetRandomNumber(id+1) * 0.5;
        float vx = cos(angle) * radius;
        float vz = sin(angle) * radius;
        float vy = 1.0 + GetRandomNumber(id+2) * 1.5;

        p.position = vec3(0.0);
        p.velocity = vec3(vx, vy, vz);
        p.lifetime = 0.1 + GetRandomNumber(id+3) * 1.0;
    } else {
        // Move particles upward
        p.position += p.velocity * pc.deltaTime;

        // Increase conical spread with slight X/Z scatter
        p.velocity.x += (GetRandomNumber(id+4) - 0.5) * 0.01;
        p.velocity.z += (GetRandomNumber(id+5) - 0.5) * 0.01;

        p.lifetime -= pc.deltaTime;
    }

    particles[id] = p;
}