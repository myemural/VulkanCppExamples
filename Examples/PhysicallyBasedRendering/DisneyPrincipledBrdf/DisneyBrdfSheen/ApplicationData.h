/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    06.06.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Material.h"
#include "SceneConfig.h"

namespace examples::physically_based_rendering::disney_principled_brdf::disney_brdf_sheen
{

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::ALBEDO_COLOR_VEC4,    common::scene::MaterialComponent::ROUGHNESS_FLOAT,
    common::scene::MaterialComponent::METALLIC_FLOAT,       common::scene::MaterialComponent::SPECULAR_STRENGTH_FLOAT,
    common::scene::MaterialComponent::SPECULAR_TINT_FLOAT,  common::scene::MaterialComponent::SUBSURFACE_FLOAT,
    common::scene::MaterialComponent::CLEARCOAT_FLOAT,      common::scene::MaterialComponent::CLEARCOAT_GLOSS_FLOAT,
    common::scene::MaterialComponent::SHEEN_FLOAT,          common::scene::MaterialComponent::SHEEN_TINT_FLOAT,
    common::scene::MaterialComponent::ALBEDO_MAP_TEXTURE,   common::scene::MaterialComponent::ROUGHNESS_MAP_TEXTURE,
    common::scene::MaterialComponent::METALLIC_MAP_TEXTURE, common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

// Constants
inline constexpr auto kLightDirection = glm::vec3(-0.1f, -0.4f, -0.7f);
inline constexpr auto kLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
inline constexpr auto kLightIntensity = 1.0f;
inline constexpr auto kEnvironmentCubemapSize = 1024U;
inline constexpr auto kIrradianceCubemapSize = 64U;
inline constexpr auto kBrdfLutImageSize = 512U;
inline constexpr auto kLocalThreadCountX = 16U;
inline constexpr auto kLocalThreadCountY = 16U;
inline constexpr auto kPrefilterCubemapSize = 256U;
inline constexpr auto kPrefilterRoughnessMaxMipCount = 5U;
inline constexpr auto kObjectCountPerRow = 5U;
inline constexpr auto kValueIncrementPerObject = 1.0f / static_cast<float>(kObjectCountPerRow - 1);

struct alignas(16) LightUbo
{
    glm::vec4 lightDirection;         // xyz = Light Direction
    glm::vec4 lightColorAndIntensity; // xyz = Light Color, w = Light Intensity
};

struct SkyboxPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
};

struct ScenePushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
    std::uint32_t prefilterRoughnessMaxMipCount;
    std::uint32_t isIblEnabled = 1; // 0: IBL Disabled, 1: IBL Enabled
};

struct PrefilterMapPushConstants
{
    float roughness;
    std::uint32_t mipLevel;
    std::uint32_t mipSize;
};

} // namespace examples::physically_based_rendering::disney_principled_brdf::disney_brdf_sheen
