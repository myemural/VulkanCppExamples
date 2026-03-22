// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct Particle
{
    float3 position;
    float3 velocity;
    float  lifetime;
};

RWStructuredBuffer<Particle> particles;

struct PushConstants
{
    float deltaTime;
    int   particleCount;
};
[[vk::push_constant]] PushConstants pc;

float GetRandomNumber(uint seed)
{
    return frac(sin(float(seed) * 12.345) * 4567.89);
}

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    uint id = dtid.x;

    if (id >= pc.particleCount) {
        return;
    }

    Particle p = particles[id];

    // Respawn if dead or invalid
    if (p.lifetime <= 0.0 || isnan(p.lifetime))
    {
        // Conical spawn
        float angle  = GetRandomNumber(id)   * 6.2831853;
        float radius = GetRandomNumber(id+1) * 0.5;

        float vx = cos(angle) * radius;
        float vz = sin(angle) * radius;
        float vy = 1.0 + GetRandomNumber(id+2) * 1.5;

        p.position = float3(0.0, 0.0, 0.0);
        p.velocity = float3(vx, vy, vz);
        p.lifetime = 0.1 + GetRandomNumber(id+3) * 1.0;
    }
    else
    {
        // Move particles upward
        p.position += p.velocity * pc.deltaTime;

        // Increase conical spread with slight X/Z scatter
        p.velocity.x += (GetRandomNumber(id+4) - 0.5) * 0.01;
        p.velocity.z += (GetRandomNumber(id+5) - 0.5) * 0.01;

        // Lifetime decrement
        p.lifetime -= pc.deltaTime;
    }

    particles[id] = p;
}