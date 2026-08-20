/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    19.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "MathUtils.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::occlusion_culling_compute
{

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4,      common::scene::MaterialComponent::SPECULAR_COLOR_VEC4,
    common::scene::MaterialComponent::AMBIENT_STRENGTH_FLOAT,  common::scene::MaterialComponent::SHININESS_FLOAT,
    common::scene::MaterialComponent::SPECULAR_STRENGTH_FLOAT, common::scene::MaterialComponent::UV_SCALE_FLOAT,
    common::scene::MaterialComponent::DIFFUSE_MAP_TEXTURE,     common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

// Constants
inline constexpr auto kLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
inline constexpr auto kLightDirection = glm::vec3(-0.4f, -0.4f, -0.4f);
inline constexpr auto kAmbientStrength = 0.05f;
inline constexpr auto kSpecularStrength = 0.7f;
inline constexpr auto kSpecularShininess = 128.0f;
inline constexpr auto kTotalNoOfCubePositions = 900U;
inline constexpr auto kTotalNoOfPlanePositions = 2U;
inline constexpr auto kCubeScale = 1.5f;
inline constexpr auto kPlaneScale = 32.0f;
inline constexpr auto kHizBuildLocalSizeX = 16U;
inline constexpr auto kHizBuildLocalSizeY = 16U;
inline constexpr auto kCullingLocalSize = 64U;
inline constexpr auto kTotalObjectCount = kTotalNoOfCubePositions + kTotalNoOfPlanePositions + 1U; // Plus root
inline constexpr auto kFrustumCulledResultIndex = 0U;
inline constexpr auto kOcclusionCulledResultIndex = 1U;
inline constexpr auto kVisibilityArrayResultIndex = 4U;
inline constexpr auto kCullingResBufferElementCount = kVisibilityArrayResultIndex + kTotalObjectCount;


struct alignas(16) ObjectBounds
{
    glm::vec4 center; // xyz = World-space AABB center
    glm::vec4 extent; // xyz = World-space AABB half extent
};

struct alignas(16) LightUbo
{
    glm::vec4 lightDirection; // xyz = Light Direction
    glm::vec4 lightColor;     // xyz = Light Color
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
};

struct OcclusionPushConstants
{
    glm::mat4 viewProjMatrix;
    glm::vec2 hizBaseSize;
    std::uint32_t hizMipCount;
    std::uint32_t objectCount;
    std::uint32_t frustumCullingEnabled;
    std::uint32_t occlusionCullingEnabled;
};

// Generate positions for the objects and occluders
inline const std::vector<glm::vec3> cubePositions = common::utility::GenerateRandomPositions(
        kTotalNoOfCubePositions, glm::vec3(-15.0f, -32.0f, -25.0f), glm::vec3(15.0f, 32.0f, -2.0f), 2.0f);

inline constexpr std::array<glm::vec3, kTotalNoOfPlanePositions> planePositions{glm::vec3{0.0f, -8.0f, -11.5f},
                                                                                glm::vec3{0.0f, 8.0f, -11.5f}};

} // namespace examples::advanced_shader_programming::advanced_compute_shaders::occlusion_culling_compute
