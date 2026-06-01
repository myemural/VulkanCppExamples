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

#include "MathUtils.h"

namespace examples::real_time_lighting::transparency_techniques::pixel_linked_lists_transparency
{

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4,      common::scene::MaterialComponent::SPECULAR_COLOR_VEC4,
    common::scene::MaterialComponent::AMBIENT_STRENGTH_FLOAT,  common::scene::MaterialComponent::SHININESS_FLOAT,
    common::scene::MaterialComponent::SPECULAR_STRENGTH_FLOAT, common::scene::MaterialComponent::OPACITY_FLOAT,
    common::scene::MaterialComponent::DIFFUSE_MAP_TEXTURE,     common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

// Constants
inline constexpr auto kLightDirection = glm::vec3(-0.1f, -0.4f, -0.7f);
inline constexpr auto kLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
inline constexpr auto kAmbientStrength = 0.05f;
inline constexpr auto kSpecularStrength = 0.7f;
inline constexpr auto kSpecularShininess = 128.0f;
inline constexpr auto kTotalNoOfPositions = 250U;
inline constexpr auto kMaxFragmentsPerPixel = 8U;
inline constexpr std::uint32_t kNullNode = 0xFFFFFFFFu;

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

// Scene object and light position vectors
inline const std::vector<glm::vec3> modelPositions = common::utility::GenerateRandomPositions(
        kTotalNoOfPositions, glm::vec3(-15.0f, -13.0f, -25.0f), glm::vec3(15.0f, 13.0f, -2.0f), 3.0f);

} // namespace examples::real_time_lighting::transparency_techniques::pixel_linked_lists_transparency
