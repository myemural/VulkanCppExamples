/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    10.05.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::physically_based_rendering::basic_pbr::cook_torrance_microfacet
{

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::ALBEDO_COLOR_VEC4, common::scene::MaterialComponent::ROUGHNESS_FLOAT};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3)};

inline constexpr auto kLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

struct alignas(16) LightUbo
{
    glm::vec4 lightPosition; // xyz = Light Position
    glm::vec4 lightColor;    // xyz = Light Color
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
};

} // namespace examples::physically_based_rendering::basic_pbr::cook_torrance_microfacet
