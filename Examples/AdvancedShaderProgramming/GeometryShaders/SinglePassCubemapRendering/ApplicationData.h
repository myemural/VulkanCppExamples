/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    13.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Material.h"
#include "MathUtils.h"

namespace examples::advanced_shader_programming::geometry_shaders::single_pass_cubemap_rendering
{

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4,      common::scene::MaterialComponent::SPECULAR_COLOR_VEC4,
    common::scene::MaterialComponent::AMBIENT_STRENGTH_FLOAT,  common::scene::MaterialComponent::SHININESS_FLOAT,
    common::scene::MaterialComponent::SPECULAR_STRENGTH_FLOAT, common::scene::MaterialComponent::UV_SCALE_FLOAT,
    common::scene::MaterialComponent::DIFFUSE_MAP_TEXTURE,     common::scene::MaterialComponent::NORMAL_MAP_TEXTURE,
    common::scene::MaterialComponent::FLIP_NORMALS_INT};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

// Constants
inline constexpr auto kLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
inline constexpr auto kAmbientStrength = 0.05f;
inline constexpr auto kSpecularStrength = 0.7f;
inline constexpr auto kSpecularShininess = 128.0f;
inline constexpr auto kConstantFactor = 1.0f;
inline constexpr auto kLinearFactor = 0.07f;
inline constexpr auto kQuadraticFactor = 0.0017f;
inline constexpr auto kTotalNoOfPositions = 50U;
inline constexpr auto kShadowMapSize = 1024U;
inline constexpr auto kCubemapFaceCount = 6U;
inline constexpr auto kPointLightFarPlane = 50.0f;

struct alignas(16) LightUbo
{
    glm::vec4 lightPosition;    // xyz = Light Position
    glm::vec4 lightColor;       // xyz = Light Color
    glm::vec4 pointLightParams; // x = Constant Factor, y = Linear Factor, z = Quadratic Factor
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
    glm::vec4 lightPos;
    std::uint32_t objectId;
    float farPlane;
};

struct alignas(16) LightSpaceMatricesUBO
{
    glm::mat4 lightSpaceMatrices[6]; // For each cubemap face
};

inline const std::vector<glm::vec3> kObjectPositions = common::utility::GenerateRandomPositionsOnBounds(
        kTotalNoOfPositions, glm::vec3(-7.0f, -7.0f, -7.0f), glm::vec3(7.0f, 7.0f, 7.0f), 2.0f);

} // namespace examples::advanced_shader_programming::geometry_shaders::single_pass_cubemap_rendering
