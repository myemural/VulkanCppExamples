/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    04.04.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Material.h"
#include "MathUtils.h"

namespace examples::real_time_shadows::basic_shadow_mapping::multiple_shadow_mapping
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
inline constexpr auto kLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
inline constexpr auto kAmbientStrength = 0.05f;
inline constexpr auto kSpecularStrength = 0.7f;
inline constexpr auto kSpecularShininess = 128.0f;
inline constexpr auto kConstantFactor = 1.0f;
inline constexpr auto kLinearFactor = 0.07f;
inline constexpr auto kQuadraticFactor = 0.0017f;
inline constexpr auto kInnerCutoffAngle = 12.5f;
inline constexpr auto kOuterCutoffAngle = 25.0f;
inline constexpr auto kShadowMapSize = 1024U;
inline constexpr auto kPointLightFarPlane = 50.0f;

struct alignas(16) LightUbo
{
    glm::vec4 pointLightPosition;          // xyz = Light Position
    glm::vec4 pointLightColor;             // xyz = Light Color
    glm::vec4 pointLightParams;            // x = Constant Factor, y = Linear Factor, z = Quadratic Factor

    glm::vec4 directionalLightDirection;   // xyz = Light Direction (world space, normalized)
    glm::vec4 directionalLightColor;       // xyz = Light Color
    glm::mat4 directionalLightSpaceMatrix; // Light-space matrix for directional light

    glm::vec4 spotLightPosition;           // xyz = Light Position
    glm::vec4 spotLightDirection;          // xyz = Light Direction
    glm::vec4 spotLightColor;              // xyz = Light Color
    glm::vec4 spotLightParams;             // x = cos(innerCutoffAngle), y = cos(outerCutoffAngle)
    glm::mat4 spotLightSpaceMatrix;        // Light-space matrix for spotlight
};

struct ScenePushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
    float farPlane;
};

struct ShadowMapPushConstants
{
    glm::mat4 lightSpaceMatrix;
    glm::vec4 lightPos;
    std::uint32_t objectId;
    float farPlane;
    std::uint32_t isPointLightShadow = 0; // 0: False, 1: True
};

} // namespace examples::real_time_shadows::basic_shadow_mapping::multiple_shadow_mapping
