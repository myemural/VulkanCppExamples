#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out float lifetime;

struct Particle {
    vec3 position;
    vec3 velocity;
    float lifetime;
};

layout(std430, binding = 0) readonly buffer Particles {
    Particle particles[];
};

layout(push_constant) uniform PushConstants {
    mat4 mvpMatrix;
} pc;

void main()
{
    uint id = gl_VertexIndex;
    Particle particle = particles[id];

    gl_Position = pc.mvpMatrix * vec4(particle.position, 1.0);

    gl_PointSize = 15.0 - clamp(15.0 * particle.lifetime, 0.0, 15.0);
    lifetime = particle.lifetime;
}