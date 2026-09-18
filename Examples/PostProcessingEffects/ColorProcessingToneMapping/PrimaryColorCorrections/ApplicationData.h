/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    15.09.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "AppConfig.h"
#include "BuiltinPrimitives.h"
#include "Material.h"
#include "MathUtils.h"
#include "SceneConfig.h"

namespace examples::post_processing_effects::color_processing_tone_mapping::primary_color_corrections
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

struct alignas(16) PointLightGpuData
{
    glm::vec4 lightPosition;       // xyz = Light Position (View-Space)
    glm::vec4 lightColorIntensity; // xyz = Light Color, w = Color Intensity
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    std::uint32_t objectId;
};

struct LightingPushConstants
{
    std::uint32_t lightCount;
};

struct ColorCorrectionPushConstants
{
    float brightness;                     // Range: [-0.5, 0.5], Default 0.0 (additive offset)
    float contrast;                       // Range: [0.0, 2.0], Default 1.0
    float saturation;                     // Range:[0.0, 2.0], Default 1.0
    float hueShift;                       // Range: [-180, 180], default 0.0 (in degrees)
    std::uint32_t applyInDisplayEncoding; // 0 = Linear, 1 = sRGB-encoded space
};

struct TextureAssetDesc
{
    std::string textureName;
    std::string texturePath;
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
};

struct MaterialDesc
{
    std::string materialName;
    glm::vec4 albedoColor = glm::vec4(1.0f);
    std::string albedoTextureName;
    float roughness = 0.5f;
    std::string roughnessTextureName;
    float metallic = 0.5f;
    std::string metallicTextureName;
    std::string normalTextureName;
    float uvScale = 1.0f;
};

struct SceneObjectDesc
{
    common::scene::BuiltinMeshType meshType;
    glm::vec3 position;
    glm::vec3 eulerAngles;
    glm::vec3 scale;
    std::string materialName;
};

struct ModelDesc
{
    glm::vec3 position;
    glm::vec3 eulerAngles;
    float scale;
};

// All textures in the scene
// clang-format off
inline const std::vector<TextureAssetDesc> textureAssets{
    {constants::kFloorTexture, constants::kFloorTexturePath},
    {constants::kFloorNormalTexture, constants::kFloorNormalTexturePath, VK_FORMAT_R8G8B8A8_UNORM},
    {constants::kFloorRoughnessTexture, constants::kFloorRoughnessTexturePath, VK_FORMAT_R8G8B8A8_UNORM},
    {constants::kMetalDamagedAlbedoTexture, constants::kMetalDamagedAlbedoTexturePath},
    {constants::kMetalDamagedNormalTexture, constants::kMetalDamagedNormalTexturePath, VK_FORMAT_R8G8B8A8_UNORM},
    {constants::kMetalDamagedRoughnessTexture, constants::kMetalDamagedRoughnessTexturePath, VK_FORMAT_R8G8B8A8_UNORM},
    {constants::kMetalDamagedMetallicTexture, constants::kMetalDamagedMetallicTexturePath, VK_FORMAT_R8G8B8A8_UNORM}
};
// clang-format on

// All materials in the scene
// clang-format off
inline const std::vector<MaterialDesc> materials{
    {constants::kFloorMaterial, glm::vec4(1.0f), constants::kFloorTexture,
        1.0f, constants::kFloorRoughnessTexture, 0.0f, "", constants::kFloorNormalTexture, 5.0f},
    {constants::kGreenWallMaterial, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), "",
            0.5f, "", 0.0f, ""},
    {constants::kRedWallMaterial, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), "",
            0.5f, "", 0.0f, ""},
    {constants::kWhiteWallMaterial, glm::vec4(1.0f), "",
            0.5f, "", 0.0f, ""},
    {constants::kMetalDamagedMaterial, glm::vec4(1.0f), constants::kMetalDamagedAlbedoTexture,
            0.5f, constants::kMetalDamagedRoughnessTexture, 0.5f, constants::kMetalDamagedMetallicTexture, constants::kMetalDamagedNormalTexture, 2.0f},
    {constants::kRedMaterial, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), "", 0.15f, "", 0.0f, "", "", 1.0f},
    {constants::kGreenMaterial, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), "", 0.35f, "", 0.25f, "", "", 1.0f},
    {constants::kBlueMaterial, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), "", 0.55f, "", 0.5f, "", "", 1.0f},
    {constants::kYellowMaterial, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), "", 0.70f, "", 0.75f, "", "", 1.0f},
    {constants::kMagentaMaterial, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), "", 0.85f, "", 1.0f, "", "", 1.0f},
    {constants::kCyanMaterial, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), "", 0.35f, "", 0.9f, "", "", 1.0f}
};
// clang-format on

// Point lights in the scene (should be converted to the view-space)
// clang-format off
inline const std::vector<PointLightGpuData> pointLights{
    {glm::vec4{0.0f, 3.0f, 0.0f, 1.0f}, glm::vec4{1.0f, 1.0f, 1.0f, 50.0f}}
};
// clang-format on

// Room parts (floor and three walls)
// clang-format off
inline const std::vector<SceneObjectDesc> roomParts{
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, -0.25f, 0.0f), glm::vec3(0.0f), glm::vec3(16.0f, 0.5f, 16.0f), constants::kFloorMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 4.0f, -8.25f), glm::vec3(0.0f), glm::vec3(16.5f, 8.0f, 0.5f), constants::kWhiteWallMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(-8.25f, 4.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.5f, 8.0f, 16.5f), constants::kGreenWallMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(8.25f, 4.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.5f, 8.0f, 16.5f), constants::kRedWallMaterial}
};
// clang-format on

// Objects placed inside the room (mostly close to the walls)
// clang-format off
inline const std::vector<SceneObjectDesc> propObjects{
        // Sphere pyramid in the middle of the room
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.0f, 0.5f, -3.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 0.5f, -3.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.0f, 0.5f, -3.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.0f, 0.5f, -2.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 0.5f, -2.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.0f, 0.5f, -2.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.0f, 0.5f, -1.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 0.5f, -1.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.0f, 0.5f, -1.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-0.5f, 1.207f, -2.5f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.5f, 1.207f, -2.5f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-0.5f, 1.207f, -1.5f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.5f, 1.207f, -1.5f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 1.914f, -2.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},

        // Six spheres in front of the back wall
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-5.5f, 1.0f, -6.9f), glm::vec3(0.0f), glm::vec3(2.0f), constants::kRedMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-3.3f, 1.0f, -6.9f), glm::vec3(0.0f), glm::vec3(2.0f), constants::kGreenMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.1f, 1.0f, -6.9f), glm::vec3(0.0f), glm::vec3(2.0f), constants::kBlueMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.1f, 1.0f, -6.9f), glm::vec3(0.0f), glm::vec3(2.0f), constants::kYellowMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(3.3f, 1.0f, -6.9f), glm::vec3(0.0f), glm::vec3(2.0f), constants::kMagentaMaterial},
        {common::scene::BuiltinMeshType::SPHERE, glm::vec3(5.5f, 1.0f, -6.9f), glm::vec3(0.0f), glm::vec3(2.0f), constants::kCyanMaterial},

        // Three cubes in fron of the left wall
        {common::scene::BuiltinMeshType::CUBE, glm::vec3(-6.5f, 1.0f, -1.25f), glm::vec3(0.0f), glm::vec3(2.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::CUBE, glm::vec3(-6.5f, 1.0f, 1.25f), glm::vec3(0.0f), glm::vec3(2.0f), constants::kMetalDamagedMaterial},
        {common::scene::BuiltinMeshType::CUBE, glm::vec3(-6.5f, 3.0f, 0.0f), glm::vec3(0.0f, 45.0f, 0.0f), glm::vec3(2.0f), constants::kMetalDamagedMaterial},
};
// clang-format on

// GlTF model instances
// clang-format off
inline const std::vector<ModelDesc> suzanneInstances{
        {glm::vec3(6.8f, 1.35f, -5.0f), glm::vec3(0.0f, -70.0f, 0.0f), 1.1f},
        {glm::vec3(6.8f, 1.3f, 0.0f), glm::vec3(0.0f, -70.0f, 0.0f), 1.1f},
        {glm::vec3(6.8f, 1.3f, 5.0f), glm::vec3(0.0f, -70.0f, 0.0f), 1.1f},
};
// clang-format on

} // namespace examples::post_processing_effects::color_processing_tone_mapping::primary_color_corrections
