/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    22.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "MathUtils.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::wind_simulation_foliage
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
inline constexpr auto kGrassPointCount = 800U;
inline constexpr auto kWindGridSize = 8U;
inline constexpr auto kWindCellCount = kWindGridSize * kWindGridSize;
inline constexpr auto kWindDebugLineVertexCount = kWindCellCount * 2U;
inline constexpr auto kWindDebugVolumeVertexCount = 24U;
inline constexpr auto kDebugBoxColor = glm::vec4{0.9f, 0.85f, 0.2f, 1.0f};
inline constexpr auto kBaseWindDirection = glm::vec2(1.0f, 0.4f);
inline constexpr auto kBaseStrength = 0.7f;
inline constexpr auto kGustSpeed = 0.35f;
inline constexpr auto kNoiseScale = 0.35f;

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

struct WindUbo
{
    glm::vec4 volumeMin;        // xyz = Volume min corner position
    glm::vec4 volumeMax;        // xyz = Volume max corner position
    glm::vec4 windDirTimeAlpha; // xy = Base wind direction (xz), z = Time (seconds), w  = Unused
    glm::vec4 params;           // x = Base strength, y = Gust speed, z = Noise scale, w = Grid size (float)
};

struct DebugPushConstants
{
    glm::mat4 viewProj;
};

struct LineVertexCpu
{
    glm::vec4 position;
    glm::vec4 color;
};

// Grass positions vector
inline const std::vector<glm::vec3> kGrassPositions = common::utility::GenerateRandomPositions(
        kGrassPointCount, glm::vec3(-3.5f, -2.0f, -3.5f), glm::vec3(3.5f, -2.0f, 3.5f), 1.0f);
inline constexpr glm::vec3 kWindVolumeMin{-4.5f, -2.0f, -4.5f};
inline constexpr glm::vec3 kWindVolumeMax{4.5f, 0.5f, 4.5f};

} // namespace examples::advanced_shader_programming::advanced_compute_shaders::wind_simulation_foliage
