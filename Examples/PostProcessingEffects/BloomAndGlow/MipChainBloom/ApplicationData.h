/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    29.10.2026
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

namespace examples::post_processing_effects::bloom_and_glow::mip_chain_bloom
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

// Constants
inline constexpr auto kLightSphereRadius = 0.5f;
inline constexpr auto kBloomMipCount = 6U;       // Number of mip levels of the bloom chain
inline constexpr auto kBloomFilterRadius = 1.0f; // Radius of the 3x3 tent filter used in upsample passes

inline constexpr auto kDebugViewFinal = 0U;
inline constexpr auto kDebugViewBloomOnly = 1U;
inline constexpr auto kDebugViewCount = 2U;

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
    std::int32_t lightIndex; // -1: Regular scene object, >= 0: Index of the light
};

struct LightingPushConstants
{
    std::uint32_t lightCount;
    float lightSphereRadius; // Radius of the spheres which represent the point lights in the scene
};

struct BloomPassPushConstants
{
    glm::vec2 srcTexelSize;        // 1.0 / source image size
    glm::vec2 dstTexelSize;        // 1.0 / target mip size
    std::uint32_t useKarisAverage; // Only for the first downsample pass (firefly suppression)
    float filterRadius;            // Only for upsample passes (tent filter radius in source texels)
};

struct CompositePushConstants
{
    float exposure;               // Manual exposure in EV
    float bloomStrength;          // Linear interpolation factor between the scene color and bloom
    float bloomNormalization;     // 1 / kBloomMipCount, keeps the summed mip chain energy conserving
    std::uint32_t isBloomEnabled; // 0: Bloom disabled (only tone mapping), 1: Bloom enabled
    std::uint32_t debugView;      // 0: Final image, 1: Bloom only
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
    {constants::kMetalDamagedMaterial, glm::vec4(1.0f), constants::kMetalDamagedAlbedoTexture,
            0.5f, constants::kMetalDamagedRoughnessTexture, 0.5f, constants::kMetalDamagedMetallicTexture, constants::kMetalDamagedNormalTexture, 2.0f},
    {constants::kRedMaterial, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), "", 0.15f, "", 0.0f, "", "", 1.0f},
    {constants::kGreenMaterial, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), "", 0.35f, "", 0.25f, "", "", 1.0f},
    {constants::kBlueMaterial, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), "", 0.55f, "", 0.5f, "", "", 1.0f},
    {constants::kYellowMaterial, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), "", 0.70f, "", 0.75f, "", "", 1.0f},
    {constants::kMagentaMaterial, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), "", 0.85f, "", 1.0f, "", "", 1.0f},
    {constants::kCyanMaterial, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), "", 0.35f, "", 0.9f, "", "", 1.0f},
    {constants::kLightObjectMaterial, glm::vec4(1.0f), "",
        0.5f, "", 0.0f, ""},

    // HDR Test grid materials
    {"HdrGridMat_R05_M1", glm::vec4(1.0f), "", 0.05f, "", 1.0f, "", "", 1.0f},
    {"HdrGridMat_R20_M0", glm::vec4(1.0f), "", 0.20f, "", 0.0f, "", "", 1.0f},
    {"HdrGridMat_R35_M1", glm::vec4(1.0f), "", 0.35f, "", 1.0f, "", "", 1.0f},
    {"HdrGridMat_R50_M0", glm::vec4(1.0f), "", 0.50f, "", 0.0f, "", "", 1.0f},
    {"HdrGridMat_R65_M1", glm::vec4(1.0f), "", 0.65f, "", 1.0f, "", "", 1.0f},
    {"HdrGridMat_R80_M0", glm::vec4(1.0f), "", 0.80f, "", 0.0f, "", "", 1.0f},
    {"HdrGridMat_R95_M1", glm::vec4(1.0f), "", 0.95f, "", 1.0f, "", "", 1.0f},
    {"HdrGridMat_R05_M0", glm::vec4(1.0f), "", 0.05f, "", 0.0f, "", "", 1.0f}
};
// clang-format on

// Point lights in the scene (should be converted to the view-space)
// clang-format off
inline const std::vector<PointLightGpuData> pointLights{
    // Dark room
    {glm::vec4{0.0f, 6.0f, 12.0f, 1.0f},  glm::vec4{1.0f, 0.85f, 0.65f, 8.0f}},
    {glm::vec4{-6.0f, 1.8f, 17.5f, 1.0f},   glm::vec4{1.0f, 0.6f, 0.3f, 4.5f}},

    // Bright room
    {glm::vec4{0.0f, 6.0f, -15.0f, 1.0f},  glm::vec4{1.0f, 0.97f, 0.92f, 200.0f}}
};
// clang-format on

// Room parts (floor and three walls)
// clang-format off
inline const std::vector<SceneObjectDesc> roomParts{
    // Floors
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, -0.25f, 10.0f), glm::vec3(0.0f), glm::vec3(16.5f, 0.5f, 20.0f), constants::kFloorMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, -0.25f, -10.0f), glm::vec3(0.0f), glm::vec3(16.5f, 0.5f, 20.0f), constants::kFloorMaterial},

    // Ceiling
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 8.25f, 0.0f), glm::vec3(0.0f), glm::vec3(16.5f, 0.5f, 41.0f), constants::kFloorMaterial},

    // Dark room walls (left, right, front)
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(-8.25f, 4.0f, 10.0f), glm::vec3(0.0f), glm::vec3(0.5f, 8.0f, 20.0f), constants::kFloorMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(8.25f, 4.0f, 10.0f), glm::vec3(0.0f), glm::vec3(0.5f, 8.0f, 20.0f), constants::kFloorMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 4.0f, 20.25f), glm::vec3(0.0f), glm::vec3(17.0f, 8.0f, 0.5f), constants::kFloorMaterial},

    // Bright room walls (left, right, front)
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(-8.25f, 4.0f, -10.0f), glm::vec3(0.0f), glm::vec3(0.5f, 8.0f, 20.0f), constants::kFloorMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(8.25f, 4.0f, -10.0f), glm::vec3(0.0f), glm::vec3(0.5f, 8.0f, 20.0f), constants::kFloorMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 4.0f, -20.25f), glm::vec3(0.0f), glm::vec3(17.0f, 8.0f, 0.5f), constants::kFloorMaterial},

    // Partition wall with a doorway
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(-5.0f, 4.0f, 0.0f), glm::vec3(0.0f), glm::vec3(6.0f, 8.0f, 0.5f), constants::kFloorMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(5.0f, 4.0f, 0.0f), glm::vec3(0.0f), glm::vec3(6.0f, 8.0f, 0.5f), constants::kFloorMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 6.5f, 0.0f), glm::vec3(0.0f), glm::vec3(4.0f, 3.0f, 0.5f), constants::kFloorMaterial},
};
// clang-format on

// Objects placed inside the room (mostly close to the walls)
// clang-format off
inline const std::vector<SceneObjectDesc> propObjects{
    // Sphere pyramid (in bright room)
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.0f, 0.5f, -13.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 0.5f, -13.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.0f, 0.5f, -13.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.0f, 0.5f, -12.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 0.5f, -12.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.0f, 0.5f, -12.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.0f, 0.5f, -11.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 0.5f, -11.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.0f, 0.5f, -11.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-0.5f, 1.207f, -12.5f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.5f, 1.207f, -12.5f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-0.5f, 1.207f, -11.5f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.5f, 1.207f, -11.5f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(0.0f, 1.914f, -12.0f), glm::vec3(0.0f), glm::vec3(1.0f), constants::kMetalDamagedMaterial},

    // Colored spheres (in bright room)
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-6.25f, 0.8f, -18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kRedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-3.75f, 0.8f, -18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kGreenMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.25f, 0.8f, -18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kBlueMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.25f, 0.8f, -18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kYellowMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(3.75f, 0.8f, -18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kMagentaMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(6.25f, 0.8f, -18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kCyanMaterial},

    // Colored spheres (in dark room)
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-6.25f, 0.8f, 18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kRedMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-3.75f, 0.8f, 18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kGreenMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(-1.25f, 0.8f, 18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kBlueMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(1.25f, 0.8f, 18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kYellowMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(3.75f, 0.8f, 18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kMagentaMaterial},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(6.25f, 0.8f, 18.8f), glm::vec3(0.0f), glm::vec3(1.6f), constants::kCyanMaterial},

    // Metal cubes (in dark room)
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(-6.8f, 1.0f, 5.0f), glm::vec3(0.0f), glm::vec3(2.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(-6.8f, 1.0f, 9.0f), glm::vec3(0.0f), glm::vec3(2.0f), constants::kMetalDamagedMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(-6.8f, 1.0f, 13.0f), glm::vec3(0.0f), glm::vec3(2.0f), constants::kMetalDamagedMaterial},

    // HDR test grid: 8 spheres in front of the right wall, roughness sweeping from 0.05 to 0.95, and metallic changes
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(7.2f, 0.6f, -9.0f), glm::vec3(0.0f), glm::vec3(1.2f), "HdrGridMat_R05_M1"},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(7.2f, 0.6f, -6.5f), glm::vec3(0.0f), glm::vec3(1.2f), "HdrGridMat_R20_M0"},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(7.2f, 0.6f, -4.0f), glm::vec3(0.0f), glm::vec3(1.2f), "HdrGridMat_R35_M1"},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(7.2f, 0.6f, -1.5f), glm::vec3(0.0f), glm::vec3(1.2f), "HdrGridMat_R50_M0"},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(7.2f, 0.6f, 1.5f), glm::vec3(0.0f), glm::vec3(1.2f), "HdrGridMat_R65_M1"},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(7.2f, 0.6f, 4.0f), glm::vec3(0.0f), glm::vec3(1.2f), "HdrGridMat_R80_M0"},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(7.2f, 0.6f, 6.5f), glm::vec3(0.0f), glm::vec3(1.2f), "HdrGridMat_R95_M1"},
    {common::scene::BuiltinMeshType::SPHERE, glm::vec3(7.2f, 0.6f, 9.0f), glm::vec3(0.0f), glm::vec3(1.2f), "HdrGridMat_R05_M0"},
};
// clang-format on

} // namespace examples::post_processing_effects::bloom_and_glow::mip_chain_bloom
