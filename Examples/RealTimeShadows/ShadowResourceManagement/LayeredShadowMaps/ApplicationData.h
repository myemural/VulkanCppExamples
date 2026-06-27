/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    27.06.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Material.h"
#include "MathUtils.h"

namespace examples::real_time_shadows::shadow_resource_management::layered_shadow_maps
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
inline constexpr auto kDirectionalLightDirection = glm::vec3(-0.4f, -0.4f, -0.4f);
inline constexpr auto kSpotlight1InitPosition = glm::vec3(-5.0f, 3.0f, 0.0f);
inline constexpr auto kSpotlight2InitPosition = glm::vec3(0.0f, 3.0f, 0.0f);
inline constexpr auto kSpotlight3InitPosition = glm::vec3(5.0f, 3.0f, 0.0f);
inline constexpr auto kLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
inline constexpr auto kAmbientStrength = 0.05f;
inline constexpr auto kSpecularStrength = 0.7f;
inline constexpr auto kSpecularShininess = 128.0f;
inline constexpr auto kInnerCutoffAngle = 12.5f;
inline constexpr auto kOuterCutoffAngle = 25.0f;
inline constexpr auto kShadowMapSize = 1024U;
inline constexpr auto kDirectionalLightFarPlane = 50.0f;
inline constexpr auto kNumOfLights = 4U;

enum LightType
{
    LIGHT_TYPE_DIRECTIONAL = 0,
    LIGHT_TYPE_SPOT = 1
};

struct LightProperty
{
    LightType type;
};

inline constexpr std::array kLightProperties{
    LightProperty{LIGHT_TYPE_DIRECTIONAL},
    LightProperty{LIGHT_TYPE_SPOT},
    LightProperty{LIGHT_TYPE_SPOT},
    LightProperty{LIGHT_TYPE_SPOT},
};

struct alignas(16) LightDataGpu
{
    glm::mat4 lightSpaceMatrix; // Light-space matrix
    glm::vec4 lightPosition;    // xyz = Light Position (for spotlight only)
    glm::vec4 lightDirection;   // xyz = Light Direction
    glm::vec4 lightColor;       // xyz = Light Color
    glm::vec4 lightParams;      // x = Light Type (0: Directional, 1: Spot), y = cos(innerCutoffAngle) for spotlight,
                                // z = cos(outerCutoffAngle) for spotlight
};

struct alignas(16) LightUboGpu
{
    LightDataGpu lightData[kNumOfLights];
};

struct ScenePushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
};

struct ShadowMapPushConstants
{
    glm::mat4 lightSpaceMatrix;
    std::uint32_t objectId;
};

} // namespace examples::real_time_shadows::shadow_resource_management::layered_shadow_maps
