#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec2 fragOffset;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out float fragIntensity;

const uint kGalaxyCount = 2;

struct Particle
{
    vec4 positionMass; // xyz = Position, w = Mass
    vec4 velocityType; // xyz = Velocity, w = Galaxy Index (0.0 or 1.0)
};

layout(std430, binding = 0) readonly buffer Particles {
    Particle particles[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 viewProjMatrix;
    vec2 projScale;
    float particleRadius;
    float brightness;
} pc;

vec2 kCorners[6] = vec2[](
        // Triangle 1
        vec2(-1.0, -1.0), // Bottom-left
        vec2( 1.0, -1.0), // Bottom-right
        vec2(-1.0,  1.0), // Top-left

        // Triangle 2
        vec2(-1.0,  1.0), // Top-left
        vec2( 1.0, -1.0), // Bottom-right
        vec2( 1.0,  1.0)  // Top-right
);

const vec3 kGalaxyColors[kGalaxyCount] = vec3[](vec3(1.0, 0.05, 0.0), vec3(1.0, 0.85, 0.05));
const vec3 kHotColor = vec3(0.45, 0.20, 1.0);

void main()
{
    const Particle particle = particles[gl_InstanceIndex];
    const vec2 corner = kCorners[gl_VertexIndex];

    // Speed-based heat ramp
    const float speed = length(particle.velocityType.xyz);
    const float heat = clamp(speed * 0.02, 0.0, 1.0);
    const vec3 baseColor = kGalaxyColors[uint(particle.velocityType.w)];

    fragColor = mix(baseColor, kHotColor, heat);
    fragOffset = corner;
    fragIntensity = pc.brightness * (0.55 + 0.85 * heat);

    // Heavier bodies (the galactic cores) are drawn larger
    const float radius = pc.particleRadius * (1.0 + 0.45 * log2(1.0 + particle.positionMass.w));

    // Offsetting clip-space XY by a constant is equivalent to view-space offset
    vec4 clipPosition = pc.viewProjMatrix * vec4(particle.positionMass.xyz, 1.0);
    clipPosition.xy += corner * radius *pc.projScale;

    gl_Position = clipPosition;
}
