/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    16.02.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "MathUtils.h"

namespace examples::real_time_lighting::lighting_architectures::tiled_deferred_shading
{

inline const std::vector enabledMaterialComponents{common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4,
                                                   common::scene::MaterialComponent::DIFFUSE_MAP_TEXTURE,
                                                   common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

// Constants
inline constexpr auto kTileSizeX = 16U;
inline constexpr auto kTileSizeY = 16U;
inline constexpr auto kTotalNoOfPositions = 250U;
inline constexpr auto kMaxLightCount = 50U;
inline constexpr auto kMaxLightCountPerTile = 64U;

struct alignas(16) PointLightData
{
    glm::vec4 lightPositionIntensity; // xyz = Light Position (View-Space), w = Light Intensity
    glm::vec4 lightColorRadius;       // xyz = Light Color, w = Radius
};

struct alignas(16) TileLightList
{
    uint32_t count;
    uint32_t indices[kMaxLightCountPerTile];
};

struct GeometryPipelinePushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    std::uint32_t objectId;
};

struct LightCullPipelinePushConstants
{
    glm::mat4 proj;       // Projection Matrix
    glm::vec4 screenSize; // x = Width, y = Height, zw = Unused
    std::uint32_t lightCount;
};

struct LightPassPipelinePushConstants
{
    std::uint32_t tilesX;
};

// Scene object and light position vectors
inline const std::vector<glm::vec3> modelPositions = common::utility::GenerateRandomPositions(
        kTotalNoOfPositions, glm::vec3(-15.0f, -13.0f, -25.0f), glm::vec3(15.0f, 13.0f, -2.0f), 3.0f);

} // namespace examples::real_time_lighting::lighting_architectures::tiled_deferred_shading
