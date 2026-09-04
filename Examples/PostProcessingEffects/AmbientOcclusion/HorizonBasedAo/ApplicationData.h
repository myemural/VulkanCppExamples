/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    04.09.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "BuiltinPrimitives.h"
#include "Material.h"
#include "MathUtils.h"
#include "SceneConfig.h"

namespace examples::post_processing_effects::ambient_occlusion::horizon_based_ao
{

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4, common::scene::MaterialComponent::UV_SCALE_FLOAT,
    common::scene::MaterialComponent::DIFFUSE_MAP_TEXTURE, common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

// Constants
inline constexpr auto kHbaoDirectionCount = 8U;
inline constexpr auto kHbaoStepCount = 6U;
inline constexpr auto kHbaoNoiseDim = 4U;
inline constexpr auto kLightDirection = glm::vec3(0.0f, 0.0f, -0.55f);
inline constexpr auto kLightColor = glm::vec3(0.75f, 0.75f, 0.75f);
inline constexpr auto kInitialAmbientStrength = 0.45f;
inline constexpr auto kInitialHbaoRadius = 1.5f;
inline constexpr auto kInitialHbaoAngleBias = 0.15f;
inline constexpr auto kInitialHbaoPower = 1.5f;

enum class DebugMode : std::uint32_t
{
    OFF = 0U,     // Off
    ALBEDO,       // Albedo Output
    NORMAL,       // Normalized View-Space Normal Output
    HBAO_RAW,     // Raw Occlusion Factor Output
    HBAO_BLURRED, // Blurred Occlusion Factor Output
    AMBIENT_ONLY  // Only Ambient Term (Occluded) Output
};

struct alignas(16) DirectionalLightUbo
{
    glm::vec4 lightDirection; // xyz = Light Direction
    glm::vec4 lightColor;     // xyz = Light Color
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    std::uint32_t objectId;
};

struct HbaoPushConstants
{
    glm::mat4 projection;
    float radius;    // View-space sampling radius
    float angleBias; // Minimum elevation angle (in radians) that is accepted as occlusion
    float power;
    std::uint32_t directionCount;
    std::uint32_t stepCount;
};

struct LightingPushConstants
{
    std::uint32_t debugMode;
    std::uint32_t hbaoEnabled;
    float ambientStrength;
};

struct SceneObjectDesc
{
    common::scene::BuiltinMeshType meshType;
    glm::vec3 position;
    glm::vec3 scale;
    float uvScale = 1.0f;
};

struct ModelDesc
{
    glm::vec3 position;
    glm::vec3 eulerAngles;
    float scale;
};

// Room parts (floor and three walls)
// clang-format off
inline const std::vector<SceneObjectDesc> roomParts{
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, -0.25f, 0.0f), glm::vec3(16.0f, 0.5f, 16.0f), 6.0f},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 4.0f, -8.25f), glm::vec3(16.5f, 8.0f, 0.5f), 4.0f},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(-8.25f, 4.0f, 0.0f), glm::vec3(0.5f, 8.0f, 16.5f), 4.0f},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(8.25f, 4.0f, 0.0f), glm::vec3(0.5f, 8.0f, 16.5f), 4.0f}
};
// clang-format on

// Objects placed inside the room (mostly close to the walls)
// clang-format off
inline const std::vector<SceneObjectDesc> propObjects{
        {common::scene::BuiltinMeshType::CUBE, glm::vec3(-5.5f, 1.0f, -6.8f), glm::vec3(2.0f)},
        {common::scene::BuiltinMeshType::CUBE, glm::vec3(4.6f, 0.75f, -7.4f), glm::vec3(1.5f)},
        {common::scene::BuiltinMeshType::CUBE, glm::vec3(-7.3f, 0.6f, -1.5f), glm::vec3(1.2f)},
        {common::scene::BuiltinMeshType::CUBE, glm::vec3(7.4f, 1.25f, -2.5f), glm::vec3(1.6f, 2.5f, 1.6f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-2.9f, 1.1f, -7.2f), glm::vec3(2.2f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.8f, 0.9f, -7.5f), glm::vec3(1.8f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-7.0f, 1.3f, 3.0f), glm::vec3(2.6f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(6.9f, 0.8f, 3.5f), glm::vec3(1.6f)},

        // Sphere pyramid in the middle of the room
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.0f, 0.5f, -3.0f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 0.5f, -3.0f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.0f, 0.5f, -3.0f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.0f, 0.5f, -2.0f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 0.5f, -2.0f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.0f, 0.5f, -2.0f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.0f, 0.5f, -1.0f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 0.5f, -1.0f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.0f, 0.5f, -1.0f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-0.5f, 1.207f, -2.5f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.5f, 1.207f, -2.5f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-0.5f, 1.207f, -1.5f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.5f, 1.207f, -1.5f), glm::vec3(1.0f)},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 1.914f, -2.0f), glm::vec3(1.0f)},
};
// clang-format on

// GlTF model instances
// clang-format off
inline const std::vector<ModelDesc> suzanneInstances{
        {glm::vec3(-1.0f, 1.35f, -5.6f), glm::vec3(0.0f), 1.2f},
        {glm::vec3(3.4f, 1.3f, 0.5f), glm::vec3(0.0f, -40.0f, 0.0f), 1.1f},
        {glm::vec3(-5.8f, 1.3f, 0.0f), glm::vec3(0.0f, 70.0f, 0.0f), 1.1f},
};
// clang-format on

} // namespace examples::post_processing_effects::ambient_occlusion::horizon_based_ao
