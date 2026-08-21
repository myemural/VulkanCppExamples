/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    21.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::advanced_shader_programming::advanced_compute_shaders::body_particle_compute
{

// Constants
inline constexpr auto kLocalSizeX = 256U;
inline constexpr auto kGalaxyCount = 2U;
inline constexpr auto kParticlesPerGalaxy = 8192U;
inline constexpr auto kParticleCount = kParticlesPerGalaxy * kGalaxyCount;
inline constexpr auto kGravity = 0.8f;
inline constexpr auto kSoftening = 3.5f;
inline constexpr auto kDamping = 0.99988f;
inline constexpr auto kMaxDeltaTime = 0.016f;
inline constexpr auto kTimeScale = 0.5f; // Half second for the slow simulation
inline constexpr auto kParticleMass = 1.0f;
inline constexpr auto kCoreMass = 300.0f;
inline constexpr auto kGalaxyRadius = 12.0f;
inline constexpr auto kGalaxyThickness = 0.8f;
inline constexpr auto kGalaxySeparation = 42.0f;
inline constexpr auto kApproachSpeed = 0.7f;
inline constexpr auto kParticleRadius = 0.16f;
inline constexpr auto kBrightness = 0.45f;
inline constexpr auto kCameraHeight = 22.0f;
inline constexpr auto kCameraDistance = 62.0f;
inline constexpr auto kCameraOrbitSpeed = 5.0f; // Degrees per second

struct alignas(16) Particle
{
    glm::vec4 positionMass; // xyz = Position, w = Mass
    glm::vec4 velocityType; // xyz = Velocity, w = Galaxy Index (0.0 or 1.0)
};

struct NBodyPushConstants
{
    float deltaTime;
    float gravity;   // Gravitational constant (G)
    float softening; // Plummer softening length, prevents singularities
    float damping;   // Slight velocity damping, keeps the system bounded
    std::uint32_t particleCount;
};

struct ParticleRenderPushConstants
{
    glm::mat4 viewProjMatrix;
    glm::vec2 projScale;
    float particleRadius;
    float brightness;
};

} // namespace examples::advanced_shader_programming::advanced_compute_shaders::body_particle_compute
