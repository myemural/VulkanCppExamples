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

StructuredBuffer<Particle> particles;

struct PushConstants {
    float4x4 mvpMatrix;
};
[[vk::push_constant]] PushConstants pc;

struct VSOutput
{
    float4 Position : SV_Position;
    float  PointSize : SV_PointSize;
    float  Lifetime : LIFETIME;
};

[shader("vertex")]
VSOutput main(uint vertexID : SV_VertexID)
{
    Particle particle = particles[vertexID];

    VSOutput o;
    o.Position = mul(pc.mvpMatrix, float4(particle.position, 1.0));
    o.PointSize = 15.0 - clamp(15.0 * particle.lifetime, 0.0, 15.0);
    o.Lifetime = particle.lifetime;

    return o;
}