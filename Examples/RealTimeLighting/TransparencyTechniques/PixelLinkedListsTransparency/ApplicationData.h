/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    20.03.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::real_time_lighting::transparency_techniques::pixel_linked_list_transparency
{

#define MAX_FRAGMENTS_PER_PIXEL 8

inline constexpr std::uint32_t kNullNode = 0xFFFFFFFFu;

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4,      common::scene::MaterialComponent::SPECULAR_COLOR_VEC4,
    common::scene::MaterialComponent::AMBIENT_STRENGTH_FLOAT,  common::scene::MaterialComponent::SHININESS_FLOAT,
    common::scene::MaterialComponent::SPECULAR_STRENGTH_FLOAT, common::scene::MaterialComponent::OPACITY_FLOAT,
    common::scene::MaterialComponent::DIFFUSE_MAP_TEXTURE,     common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

struct alignas(16) LightUbo
{
    glm::vec4 lightDirection; // xyz = Light Direction
    glm::vec4 lightColor;     // xyz = Light Color
};

struct OitNode
{
    uint32_t colorPacked; // Packed color data: packUnorm4x8(vec4(r,g,b,a))
    float depth;          // Depth value of the fragment
    uint32_t next;        // Index of the previous node (0xFFFFFFFF = end of the list)
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
    std::uint32_t screenWidth;
    std::uint32_t screenHeight;
};

struct ResolvePassPushConstants
{
    std::uint32_t screenWidth;
};

} // namespace examples::real_time_lighting::transparency_techniques::pixel_linked_list_transparency
