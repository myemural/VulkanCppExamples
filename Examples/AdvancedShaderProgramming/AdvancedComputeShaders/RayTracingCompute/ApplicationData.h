/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    31.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::advanced_shader_programming::advanced_compute_shaders::ray_tracing_compute
{

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::ALBEDO_COLOR_VEC4, common::scene::MaterialComponent::ROUGHNESS_FLOAT,
    common::scene::MaterialComponent::METALLIC_FLOAT, common::scene::MaterialComponent::REFLECTIVITY_FLOAT};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3)};

// Constants
inline constexpr auto kLocalSizeX = 8U;
inline constexpr auto kLocalSizeY = 8U;
inline constexpr auto kBvhMaxLeafSize = 4U;
inline constexpr auto kPrimitiveStackCount = 48U;
inline constexpr auto kPrimitiveSectorCount = 48U;

inline constexpr auto kLightCenter = glm::vec3(0.0f, 7.62f, -0.5f);
inline constexpr auto kLightHalfSize = 1.7f;
inline constexpr auto kLightColor = glm::vec3(1.0f, 0.95f, 0.86f);
inline constexpr auto kLightIntensity = 65.0f;
inline constexpr auto kLightEmission = 6.0f;
inline constexpr auto kCameraStartPosition = glm::vec3(0.0f, 3.2f, 9.5f);

struct SceneObjectDesc
{
    std::string name;
    common::scene::BuiltinMeshType meshType;
    glm::vec3 position;
    glm::vec3 eulerAngles;
    glm::vec3 scale;
    glm::vec4 albedoColor; // rgb = Albedo, a = Emission Strength
    float roughness;
    float metallic;
    float reflectivity;
};

struct alignas(16) LightUbo
{
    glm::vec4 lightPosition; // xyz = Area Light Center, w = Half Size
    glm::vec4 lightColor;    // xyz = Light Color, w = Intensity
};

struct RayTracePushConstants
{
    glm::mat4 invViewProj;
    glm::vec4 cameraPosition;
    glm::uvec4 frameInfo; // xy = Resolution, z = Frame Index
};

// Vector of scene object descriptions
// clang-format off
inline const std::vector<SceneObjectDesc> kSceneObjects = {
    {"floor", common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, -0.1f, 0.0f), {}, glm::vec3(10.0f, 0.2f, 10.0f),
        glm::vec4(0.72f, 0.7f, 0.66f, 0.0f), 0.85f, 0.0f, 0.05f},
    {"ceiling", common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 8.1f, 0.0f), {}, glm::vec3(10.0f, 0.2f, 10.0f),
    glm::vec4(0.72f, 0.7f, 0.66f, 0.0f), 0.9f, 0.0f, 0.0f},
    {"backWall", common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 4.0f, -5.1f), {}, glm::vec3(10.0f, 8.0f, 0.2f),
glm::vec4(0.72f, 0.7f, 0.66f, 0.0f), 0.9f, 0.0f, 0.0f},
    {"leftWall", common::scene::BuiltinMeshType::CUBE, glm::vec3(-5.1f, 4.0f, 0.0f), {}, glm::vec3(0.2f, 8.0f, 10.0f),
glm::vec4(0.72f, 0.12f, 0.1f, 0.0f), 0.9f, 0.0f, 0.0f},
    {"rightWall", common::scene::BuiltinMeshType::CUBE, glm::vec3(5.1f, 4.0f, 0.0f), {}, glm::vec3(0.2f, 8.0f, 10.0f),
glm::vec4(0.13f, 0.6f, 0.18f, 0.0f), 0.9f, 0.0f, 0.0f},
    {"areaLight", common::scene::BuiltinMeshType::CUBE, glm::vec3(kLightCenter.x, kLightCenter.y + 0.15f, kLightCenter.z), {},
        glm::vec3(kLightHalfSize * 2.0f, 0.1f, kLightHalfSize * 2.0f), glm::vec4(kLightColor, kLightEmission), 1.0f, 0.0f, 0.0f},

    {"mirrorSphere", common::scene::BuiltinMeshType::SPHERE, glm::vec3(2.4f, 1.6f, 0.6f), {}, glm::vec3(3.2f),
glm::vec4(0.95f, 0.93f, 0.88f, 0.0f), 0.02f, 1.0f, 1.0f},
    {"diffuseSphere", common::scene::BuiltinMeshType::SPHERE, glm::vec3(-2.6f, 1.3f, -0.4f), {}, glm::vec3(2.6f),
glm::vec4(0.25f, 0.42f, 0.85f, 0.0f), 0.75f, 0.0f, 0.08f},
    {"glossyCube", common::scene::BuiltinMeshType::CUBE, glm::vec3(-0.2f, 1.0f, -3.0f), glm::vec3(0.0f, 32.0f, 0.0f), glm::vec3(2.0f),
glm::vec4(0.85f, 0.62f, 0.2f, 0.0f), 0.25f, 0.9f, 0.8f},
    {"metalCylinder", common::scene::BuiltinMeshType::CYLINDER, glm::vec3(3.3f, 1.4f, -3.2f), {}, glm::vec3(1.8f, 2.8f, 1.8f),
glm::vec4(0.72f, 0.74f, 0.78f, 0.0f), 0.18f, 1.0f, 0.9f},
    {"matteCone", common::scene::BuiltinMeshType::CONE, glm::vec3(-3.4f, 1.2f, 2.2f), {}, glm::vec3(2.4f),
glm::vec4(0.8f, 0.3f, 0.55f, 0.0f), 0.9f, 0.0f, 0.03f}
};
// clang-format on
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::ray_tracing_compute
