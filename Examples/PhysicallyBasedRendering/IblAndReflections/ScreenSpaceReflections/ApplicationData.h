/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    31.05.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Material.h"
#include "MathUtils.h"
#include "SceneConfig.h"

namespace examples::physically_based_rendering::ibl_and_reflections::screen_space_reflections
{

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::ALBEDO_COLOR_VEC4,    common::scene::MaterialComponent::ROUGHNESS_FLOAT,
    common::scene::MaterialComponent::METALLIC_FLOAT,       common::scene::MaterialComponent::UV_SCALE_FLOAT,
    common::scene::MaterialComponent::ALBEDO_MAP_TEXTURE,   common::scene::MaterialComponent::ROUGHNESS_MAP_TEXTURE,
    common::scene::MaterialComponent::METALLIC_MAP_TEXTURE, common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

inline constexpr auto kLightCount = 4U;

struct alignas(16) PointLightData
{
    glm::vec4 lightPosition;          // xyz = Light Position (View-Space)
    glm::vec4 lightColorAndIntensity; // xyz = Light Color, w = Light Intensity
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    std::uint32_t objectId;
};

struct SsrPushConstants
{
    glm::mat4 proj = glm::mat4(1.0f);
    float maxDistance = 15.0f;          // Maximum ray distance in view-space
    float thickness = 0.5f;             // Depth tolerance (reduces false positives)
    std::int32_t maxSteps = 64;         // Ray march max step count
    std::int32_t binarySearchSteps = 8; // Step count for intersection refinement
    std::uint32_t isSsrEnabled = 1;     // 0: SSR Disabled, 1: SSR Enabled
};

} // namespace examples::physically_based_rendering::ibl_and_reflections::screen_space_reflections
