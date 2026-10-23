/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    23.10.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Material.h"
#include "SceneConfig.h"

namespace examples::physically_based_rendering::disney_bsdf_transmission::disney_bsdf_thin_surface
{

inline const std::vector enabledMaterialComponents{common::scene::MaterialComponent::ALBEDO_COLOR_VEC4,
                                                   common::scene::MaterialComponent::TRANSMITTANCE_COLOR_VEC4,
                                                   common::scene::MaterialComponent::ROUGHNESS_FLOAT,
                                                   common::scene::MaterialComponent::METALLIC_FLOAT,
                                                   common::scene::MaterialComponent::SPECULAR_TINT_FLOAT,
                                                   common::scene::MaterialComponent::ANISOTROPIC_FLOAT,
                                                   common::scene::MaterialComponent::SHEEN_FLOAT,
                                                   common::scene::MaterialComponent::SHEEN_TINT_FLOAT,
                                                   common::scene::MaterialComponent::CLEARCOAT_FLOAT,
                                                   common::scene::MaterialComponent::CLEARCOAT_GLOSS_FLOAT,
                                                   common::scene::MaterialComponent::IOR_FLOAT,
                                                   common::scene::MaterialComponent::SPEC_TRANS_FLOAT,
                                                   common::scene::MaterialComponent::AT_DISTANCE_FLOAT,
                                                   common::scene::MaterialComponent::THICKNESS_FLOAT,
                                                   common::scene::MaterialComponent::DIFF_TRANS_FLOAT,
                                                   common::scene::MaterialComponent::FLATNESS_FLOAT,
                                                   common::scene::MaterialComponent::UV_SCALE_FLOAT,
                                                   common::scene::MaterialComponent::ALBEDO_MAP_TEXTURE,
                                                   common::scene::MaterialComponent::ROUGHNESS_MAP_TEXTURE,
                                                   common::scene::MaterialComponent::METALLIC_MAP_TEXTURE,
                                                   common::scene::MaterialComponent::NORMAL_MAP_TEXTURE,
                                                   common::scene::MaterialComponent::THIN_SURFACE_INT};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

// Constants
inline constexpr auto kLightCount = 6U;
inline constexpr auto kLightObjectScale = 0.25f;
inline constexpr auto kEnvironmentCubemapSize = 1024U;
inline constexpr auto kIrradianceCubemapSize = 64U;
inline constexpr auto kBrdfLutImageSize = 512U;
inline constexpr auto kLocalThreadCountX = 16U;
inline constexpr auto kLocalThreadCountY = 16U;
inline constexpr auto kPrefilterCubemapSize = 256U;
inline constexpr auto kPrefilterRoughnessMaxMipCount = 5U;
inline constexpr auto kObjectCountPerRow = 5U;
inline constexpr auto kValueIncrementPerObject = 1.0f / static_cast<float>(kObjectCountPerRow - 1);

inline constexpr auto kIorMin = 1.2f;
inline constexpr auto kIorMax = 2.0f;
inline constexpr auto kIorIncrementPerObject = (kIorMax - kIorMin) / static_cast<float>(kObjectCountPerRow - 1);
inline constexpr auto kReferenceIor = 1.5f;
inline constexpr auto kDiffTransMax = 2.0f;
inline constexpr auto kCardSize = 2.4f;
inline constexpr auto kSpecularCardColor = glm::vec4(0.95f, 0.85f, 0.85f, 1.0f);
inline constexpr auto kDiffuseCardColor = glm::vec4(0.85f, 0.85f, 0.85f, 1.0f);


struct alignas(16) PointLightData
{
    glm::vec4 lightPosition;          // xyz = Light Position (World-Space)
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
    std::uint32_t isIblEnabled = 1;     // 0: IBL Disabled, 1: IBL Enabled
    std::uint32_t isTonemapEnabled = 1; // 0: Tonemap Disabled, 1: Tonemap Enabled
};

struct PrefilterMapPushConstants
{
    float roughness;
    std::uint32_t mipLevel;
    std::uint32_t mipSize;
};

struct PointLightConfig
{
    glm::vec3 color;
    float intensity;
    glm::vec3 center;
    glm::vec3 orbitRadii; // x = Semi-axis along X, y = Vertical bobbing amplitude, z = Semi-axis along Z
    float orbitSpeed;     // Angular speed in radians per second, the sign gives the direction
    float orbitPhase;     // Start angle in radians
};

// clang-format off
inline constexpr std::array<PointLightConfig, kLightCount> kPointLights = {{
    {glm::vec3{1.0f, 0.92f, 0.82f}, 120.0f, glm::vec3(-6.0f, 6.0f, 9.0f), glm::vec3(0.0f), 0.0f, 0.0f},
    {glm::vec3{0.55f, 0.7f, 1.0f}, 30.0f, glm::vec3(0.0f, 7.5f, -2.5f), glm::vec3(0.0f), 0.0f, 0.0f},
    {glm::vec3{1.0f, 0.12f, 0.08f}, 8.0f, glm::vec3(0.0f, -1.5f, 0.0f), glm::vec3(8.5f, 0.6f, 2.0f), 0.45f, 0.0f},
    {glm::vec3{0.2f, 1.0f, 0.25f}, 8.0f, glm::vec3(0.0f, -4.5f, 0.0f), glm::vec3(8.5f, 0.6f, 2.2f), -0.35f, -1.5f},
    {glm::vec3{0.1f, 0.8f, 1.0f}, 8.0f, glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(8.5f, 1.2f, 2.0f), 0.4f, 3.14f},
    {glm::vec3{1.0f, 0.6f, 0.1f}, 8.0f, glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(8.5f, 0.5f, 2.4f), -0.28f, 4.5f}
}};
// clang-format on

} // namespace examples::physically_based_rendering::disney_bsdf_transmission::disney_bsdf_thin_surface
