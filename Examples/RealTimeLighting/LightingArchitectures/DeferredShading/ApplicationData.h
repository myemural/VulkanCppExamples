/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    04.02.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Material.h"
#include "MathUtils.h"
#include "SceneConfig.h"

namespace examples::real_time_lighting::lighting_architectures::deferred_shading
{

enum class DebugMode : std::uint32_t
{
    OFF = 0U, // Off
    ALBEDO,   // Albedo Output
    POSITION, // Normalized View-Space Position Output
    NORMAL    // Normalized View-Space Normal Output
};

inline const std::vector enabledMaterialComponents{common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4,
                                                   common::scene::MaterialComponent::DIFFUSE_MAP_TEXTURE,
                                                   common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

// Constants
inline constexpr auto kTotalNoOfPositions = 250U;
inline constexpr auto kMaxLightCount = 50U;

struct alignas(16) PointLightData
{
    glm::vec4 lightPosition;    // xyz = Light Position (View-Space)
    glm::vec4 lightColor;       // xyz = Light Color
    glm::vec4 pointLightParams; // x = Constant Factor, y = Linear Factor, z = Quadratic Factor
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    std::uint32_t objectId;
};

struct LightingPushConstants
{
    std::uint32_t debugMode;
};

// Scene object and light position vectors
inline const std::vector<glm::vec3> modelPositions = common::utility::GenerateRandomPositions(
        kTotalNoOfPositions, glm::vec3(-15.0f, -13.0f, -25.0f), glm::vec3(15.0f, 13.0f, -2.0f), 3.0f);

} // namespace examples::real_time_lighting::lighting_architectures::deferred_shading
