/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    17.05.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::physically_based_rendering::textured_pbr::ambient_occlusion_pbr
{

inline const std::vector enabledMaterialComponents{common::scene::MaterialComponent::ALBEDO_COLOR_VEC4,
                                                   common::scene::MaterialComponent::ROUGHNESS_FLOAT,
                                                   common::scene::MaterialComponent::METALLIC_FLOAT,
                                                   common::scene::MaterialComponent::ALBEDO_MAP_TEXTURE,
                                                   common::scene::MaterialComponent::ROUGHNESS_MAP_TEXTURE,
                                                   common::scene::MaterialComponent::METALLIC_MAP_TEXTURE,
                                                   common::scene::MaterialComponent::NORMAL_MAP_TEXTURE,
                                                   common::scene::MaterialComponent::AMBIENT_OCCLUSION_MAP_TEXTURE};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

struct alignas(16) LightUbo
{
    glm::vec4 lightDirection;         // xyz = Light Direction
    glm::vec4 lightColorAndIntensity; // xyz = Light Color, w = Light Intensity
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
};

} // namespace examples::physically_based_rendering::textured_pbr::ambient_occlusion_pbr
