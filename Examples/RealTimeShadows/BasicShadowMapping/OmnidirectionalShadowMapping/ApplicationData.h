/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    29.03.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Material.h"
#include "MathUtils.h"

namespace examples::real_time_shadows::basic_shadow_mapping::omnidirectional_shadow_mapping
{

#define SHADOW_MAP_SIZE 1024
#define NUM_OBJECTS 50

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
    glm::mat4 lightSpaceMatrix;
    glm::vec4 lightPos;
    std::uint32_t objectId;
    float farPlane;
};

inline const std::vector<glm::vec3> kObjectPositions = common::utility::GenerateRandomPositionsOnBounds(
        NUM_OBJECTS, glm::vec3(-7.0f, -7.0f, -7.0f), glm::vec3(7.0f, 7.0f, 7.0f), 2.0f);

inline constexpr float kPointLightFarPlane = 50.0f;

} // namespace examples::real_time_shadows::basic_shadow_mapping::omnidirectional_shadow_mapping
