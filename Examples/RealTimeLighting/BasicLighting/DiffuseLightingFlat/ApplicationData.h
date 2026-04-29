/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    13.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::real_time_lighting::basic_lighting::diffuse_lighting_flat
{

inline const std::vector enabledMaterialComponents{common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3)};

struct alignas(16) LightUbo
{
    glm::vec4 lightPosition; // xyz = Light Position
    glm::vec4 lightColor;    // xyz = Light Color
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    std::uint32_t objectId;
};

} // namespace examples::real_time_lighting::basic_lighting::diffuse_lighting_flat
