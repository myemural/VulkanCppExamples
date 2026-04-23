/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    19.04.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <array>

#include <glm/glm.hpp>

#include "Material.h"

namespace examples::real_time_shadows::shadow_map_splitting_warping::cascaded_shadow_maps
{

#define SHADOW_MAP_SIZE 1024
#define NUM_CASCADES 4

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4,      common::scene::MaterialComponent::SPECULAR_COLOR_VEC4,
    common::scene::MaterialComponent::AMBIENT_STRENGTH_FLOAT,  common::scene::MaterialComponent::SHININESS_FLOAT,
    common::scene::MaterialComponent::SPECULAR_STRENGTH_FLOAT, common::scene::MaterialComponent::UV_SCALE_FLOAT,
    common::scene::MaterialComponent::DIFFUSE_MAP_TEXTURE,     common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

enum class DebugMode : std::uint32_t
{
    OFF = 0U, // Off
    ON,       // On
};

struct alignas(16) LightUbo
{
    glm::vec4 lightDirection;                   // xyz = Light Direction
    glm::vec4 lightColor;                       // xyz = Light Color
    glm::mat4 lightSpaceMatrices[NUM_CASCADES]; // Light-space matrices for each cascade
    glm::vec4 cascadeSplits;                    // xyzw = 4 splits (view space)
};

struct ScenePushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
    std::uint32_t debugMode;
};

struct ShadowMapPushConstants
{
    glm::mat4 lightSpaceMatrix;
    std::uint32_t objectId;
};

struct CascadeInfo
{
    float cascadeSplit;
    glm::mat4 lightSpaceMatrix;
};

struct FrustumPoints
{
    std::array<glm::vec3, 4> nearPlanePoints;
    std::array<glm::vec3, 4> farPlanePoints;

    [[nodiscard]] glm::vec3 GetCenterPoint() const
    {
        glm::vec3 center(0.0f);
        auto pointCount = 0;
        for (auto i = 0U; i < 4U; ++i) {
            center += nearPlanePoints[i];
            center += farPlanePoints[i];
            pointCount += 2;
        }
        center /= pointCount;
        return center;
    }
};

} // namespace examples::real_time_shadows::shadow_map_splitting_warping::cascaded_shadow_maps
