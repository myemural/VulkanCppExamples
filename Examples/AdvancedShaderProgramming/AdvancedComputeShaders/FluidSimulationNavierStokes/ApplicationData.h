/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    29.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <array>

#include <glm/glm.hpp>

#include "Material.h"
#include "MathUtils.h"
#include "SceneConfig.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_navier_stokes
{

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4,      common::scene::MaterialComponent::SPECULAR_COLOR_VEC4,
    common::scene::MaterialComponent::AMBIENT_STRENGTH_FLOAT,  common::scene::MaterialComponent::SHININESS_FLOAT,
    common::scene::MaterialComponent::SPECULAR_STRENGTH_FLOAT, common::scene::MaterialComponent::OPACITY_FLOAT};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3)};

// Constants
inline constexpr auto kGridSizeX = 64U;
inline constexpr auto kGridSizeY = 128U;
inline constexpr auto kGridSizeZ = 64U;
inline constexpr auto kLocalSizeX = 8U;
inline constexpr auto kLocalSizeY = 8U;
inline constexpr auto kLocalSizeZ = 4U;
inline constexpr auto kJacobianIterations = 20U; // Jacobi sweeps for the pressure solve, must be even

inline constexpr auto kGroundLevel = 0.0f;
inline constexpr auto kGroundSize = 14.0f;
inline constexpr auto kVolumeMin = glm::vec3(-1.0f, 0.02f, -1.0f);
inline constexpr auto kVolumeMax = glm::vec3(1.0f, 4.02f, 1.0f);
inline constexpr auto kCellSize = (kVolumeMax.x - kVolumeMin.x) / static_cast<float>(kGridSizeX);
inline constexpr auto kLightDirection = glm::vec3(-0.45f, -0.8f, -0.35f);
inline constexpr auto kLightColor = glm::vec3(1.0f, 0.96f, 0.9f);
inline constexpr auto kTimeStep = 1.0f / 60.0f; // Fixed, so the simulation doesn't depend on the frame rate
inline constexpr auto kBuoyancy = 1.8f;         // How strongly the host smoke rises
inline constexpr auto kDissipation = 0.9993f;    // Per step density fade
inline constexpr auto kSmokeAbsorption = 50.0f; // Higher values make the smoke look thicker

inline constexpr auto kOrbitTarget = glm::vec3(0.0f, 1.2f, 0.0f);
inline constexpr auto kOrbitDistance = 7.0f;
inline constexpr auto kOrbitYaw = 90.0f;
inline constexpr auto kOrbitPitch = 12.0f;
inline constexpr auto kOrbitMinDistance = 1.5f;
inline constexpr auto kOrbitMaxDistance = 30.0f;

struct alignas(16) SceneUbo
{
    glm::vec4 lightDirection; // xyz = Light Direction
    glm::vec4 lightColor;     // xyz = Light Color
    glm::vec4 volumeMin;      // xyz = Smoke Volume Min Corner, w = Smoke Absorption
    glm::vec4 volumeMax;      // xyz = Smoke Volume Max Corner
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
};

struct FluidPushConstants
{
    glm::vec4 params;    // x = Time Step, y = Buoyancy, z = Dissipation, w = Cell Size
    glm::uvec4 gridInfo; // xyz = Grid Resolution, w = 1 when the field must be seeded
};

inline constexpr VkMemoryBarrier kComputeBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT,
                                                 VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT};

inline constexpr VkMemoryBarrier kRenderBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT,
                                                VK_ACCESS_SHADER_READ_BIT};

} // namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_navier_stokes
