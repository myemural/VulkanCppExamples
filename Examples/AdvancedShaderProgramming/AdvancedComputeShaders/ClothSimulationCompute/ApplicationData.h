/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    24.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::advanced_shader_programming::advanced_compute_shaders::cloth_simulation_compute
{

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4,      common::scene::MaterialComponent::SPECULAR_COLOR_VEC4,
    common::scene::MaterialComponent::AMBIENT_STRENGTH_FLOAT,  common::scene::MaterialComponent::SHININESS_FLOAT,
    common::scene::MaterialComponent::SPECULAR_STRENGTH_FLOAT, common::scene::MaterialComponent::UV_SCALE_FLOAT,
    common::scene::MaterialComponent::DIFFUSE_MAP_TEXTURE,     common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

// Constants
inline constexpr auto kLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
inline constexpr auto kLightDirection = glm::vec3(-0.4f, -0.4f, -0.4f);
inline constexpr auto kClothGridSize = 32U;                      // Quad count per side
inline constexpr auto kClothVertexPerSide = kClothGridSize + 1U; // Particle count per side
inline constexpr auto kClothParticleCount = kClothVertexPerSide * kClothVertexPerSide;
inline constexpr auto kClothLocalSize = 8U;                      // Compute shader local size X and Y
inline constexpr auto kClothGroupCount = (kClothVertexPerSide + kClothLocalSize - 1U) / kClothLocalSize;
inline constexpr auto kClothSize = 5.0f;
inline constexpr auto kClothSpawnPosition = glm::vec3(0.0f, 3.0f, 0.0f);
inline constexpr auto kClothRestDistance = kClothSize / static_cast<float>(kClothGridSize);
inline constexpr auto kSpherePosition = glm::vec3(0.0f, 0.0f, 0.0f);
inline constexpr auto kSphereScale = 3.0f;
inline constexpr auto kSphereRadius = kSphereScale * 0.5f;
inline constexpr auto kGroundLevel = -3.0f;
inline constexpr auto kGroundScale = 14.0f;

inline constexpr auto kFixedTimeStep = 1.0f / 60.0f;  // Fixed step keeps the simulation deterministic
inline constexpr auto kSubStepCount = 4U;
inline constexpr auto kConstraintIterationCount = 8U; // Must be even to keep the ping-pong parity
inline constexpr auto kGravity = glm::vec3(0.0f, -9.81f, 0.0f);
inline constexpr auto kVelocityDamping = 0.995f;
inline constexpr auto kCollisionFriction = 0.4f;
inline constexpr auto kClothThickness = 0.03f;
inline constexpr auto kStructuralStiffness = 1.0f;
inline constexpr auto kShearStiffness = 0.6f;
inline constexpr auto kBendStiffness = 0.2f;

struct alignas(16) LightUbo
{
    glm::vec4 lightDirection; // xyz = Light Direction
    glm::vec4 lightColor;     // xyz = Light Color
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
};

struct alignas(16) ClothUbo
{
    glm::vec4 gravityDeltaTime;   // xyz = Gravity, w = Sub-step Delta Time
    glm::vec4 sphereCenterRadius; // xyz = Sphere Center, w = Sphere Radius
    glm::vec4 params;             // x = Rest Distance, y = Damping, z = Friction, w = Ground Level
    glm::vec4 stiffness;          // x = Thickness, y = Structural, z = Shear, w = Bend
    glm::uvec4 gridInfo;          // x = Vertices per Side, y = Particle Count
};

struct ClothParticle
{
    glm::vec4 position;     // xyz = Current Position, w = Inverse Mass (0 = Pinned)
    glm::vec4 prevPosition; // xyz = Position at the beginning of the sub-step
    glm::vec4 velocity;     // xyz = Velocity
    glm::vec4 normal;       // xyz = Smooth Normal
};

struct ClothVertex
{
    glm::vec4 position; // xyz = World-Space Position
    glm::vec4 normal;   // xyz = World-Space Normal
};

} // namespace examples::advanced_shader_programming::advanced_compute_shaders::cloth_simulation_compute
