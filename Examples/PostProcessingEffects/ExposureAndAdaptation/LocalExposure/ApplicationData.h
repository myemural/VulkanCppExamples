/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    05.10.2026
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

namespace examples::post_processing_effects::exposure_and_adaptation::local_exposure
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

enum class ToneMappingMode : std::uint32_t
{
    REINHARD = 0U, // Simple Reinhard (Default)
    ACES,          // ACES (Narkowicz fit)
    UCHIMURA,      // Uchimura (Gran Turismo style)
    AGX            // AgX (fast approximation)
};

// Constants

inline constexpr auto kAutoExposureKeyValue = 0.18f;
inline constexpr auto kAutoExposureMinEv = -8.0f;
inline constexpr auto kAutoExposureMaxEv = 8.0f;

inline constexpr auto kHistogramThreadCount = 16U;
inline constexpr auto kHistogramBinCount = 256U;
inline constexpr auto kHistogramMinLogLuminance = -10.0f;
inline constexpr auto kHistogramMaxLogLuminance = 8.0f;
inline constexpr auto kHistogramLowPercentile = 0.1f;
inline constexpr auto kHistogramHighPercentile = 0.9f;
inline constexpr auto kDarkToBrightAdaptationSpeed = 3.0f; // Fast adaptation to bright
inline constexpr auto kBrightToDarkAdaptationSpeed = 1.0f; // Slower adaptation to dark
inline constexpr auto kMaxAdaptationDeltaTime = 0.1f;

inline constexpr auto kLocalExposureDownSampleFactor = 32U;
inline constexpr auto kBilateralGridDepth = 16U;
inline constexpr auto kLocalExposureFilterThreadCount = 8U;
inline constexpr auto kLocalExposureContrastScale = 0.6f;
inline constexpr auto kLocalExposureBlurredLuminanceBlend = 0.4f;
inline constexpr auto kLocalExposureDetailStrength = 1.0f;
inline constexpr auto kLocalExposureKernelSizePercent = 0.5f;

struct alignas(16) PointLightGpuData
{
    glm::vec4 lightPosition;       // xyz = Light Position (View-Space)
    glm::vec4 lightColorIntensity; // xyz = Light Color, w = Color Intensity
};

struct alignas(16) AutoExposureGpuData
{
    float averageLuminance;    // Adapted luminance (linear)
    float exposureEv;          // Read by the tone mapping pass
    float targetLogLuminance;  // This frame's metered value
    float adaptedLogLuminance; // Persistent adaptation state
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

struct AutoExposurePushConstants
{
    float minLogLuminance;
    float logLuminanceRange;
    float lowPercentile;
    float highPercentile;
    float keyValue;
    float minExposureEv;
    float maxExposureEv;
    float deltaTime;
    float darkToBrightAdaptationSpeed;
    float brightToDarkAdaptationSpeed;
    std::uint32_t resetAdaptation; // For the reset of the first frame
};

struct LocalExposureSetupPushConstants
{
    float minLogLuminance;
    float logLuminanceRange;
};

struct LocalExposureFilterPushConstants
{
    std::uint32_t blurRadius;
    float blurSigma;
};

struct ToneMappingPushConstants
{
    std::uint32_t toneMappingMode;
    float middleGrey;                   // Same as the auto exposure key value (0.18)
    float highlightContrastScale;       // < 1 compresses bright surroundings
    float shadowContrastScale;          // < 1 lifts dark surroundings
    float detailStrength;               // 1 keeps local detail, > 1 boosts it
    float blurredLuminanceBlend;        // 0 = bilateral grid only, 1 = blurred luminance only
    float gridMinLogLuminance;
    float gridLogLuminanceRange;
    std::uint32_t localExposureEnabled; // 1 = Enabled, 0 = Disabled
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
    {constants::kCyanMaterial, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), "", 0.35f, "", 0.9f, "", "", 1.0f},

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
    {glm::vec4{0.0f, 7.0f, 12.0f, 1.0f},  glm::vec4{1.0f, 0.85f, 0.65f, 4.0f}},
    {glm::vec4{-6.0f, 1.8f, 17.5f, 1.0f},   glm::vec4{1.0f, 0.6f, 0.3f, 1.5f}},

    // Bright room
    {glm::vec4{0.0f, 5.0f, -15.0f, 1.0f},  glm::vec4{1.0f, 0.97f, 0.92f, 500.0f}},
    {glm::vec4{-5.0f, 4.0f, -18.5f, 1.0f},  glm::vec4{0.9f, 0.95f, 1.0f, 200.0f}},
    {glm::vec4{5.0f, 4.0f, -18.0f, 1.0f},    glm::vec4{0.9f, 0.95f, 1.0f, 200.0f}}
};
// clang-format on

// Room parts (floor and three walls)
// clang-format off
inline const std::vector<SceneObjectDesc> roomParts{
    // Floors
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, -0.25f, 10.0f), glm::vec3(0.0f), glm::vec3(16.5f, 0.5f, 20.0f), constants::kFloorMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, -0.25f, -10.0f), glm::vec3(0.0f), glm::vec3(16.5f, 0.5f, 20.0f), constants::kFloorMaterial},

    // Ceiling
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 8.25f, 0.0f), glm::vec3(0.0f), glm::vec3(16.5f, 0.5f, 41.0f), constants::kWhiteWallMaterial},

    // Dark room walls (left, right, front)
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(-8.25f, 4.0f, 10.0f), glm::vec3(0.0f), glm::vec3(0.5f, 8.0f, 20.0f), constants::kGreenWallMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(8.25f, 4.0f, 10.0f), glm::vec3(0.0f), glm::vec3(0.5f, 8.0f, 20.0f), constants::kRedWallMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 4.0f, 20.25f), glm::vec3(0.0f), glm::vec3(17.0f, 8.0f, 0.5f), constants::kWhiteWallMaterial},

    // Bright room walls (left, right, front)
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(-8.25f, 4.0f, -10.0f), glm::vec3(0.0f), glm::vec3(0.5f, 8.0f, 20.0f), constants::kGreenWallMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(8.25f, 4.0f, -10.0f), glm::vec3(0.0f), glm::vec3(0.5f, 8.0f, 20.0f), constants::kRedWallMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 4.0f, -20.25f), glm::vec3(0.0f), glm::vec3(17.0f, 8.0f, 0.5f), constants::kWhiteWallMaterial},

    // Partition wall with a doorway
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(-5.0f, 4.0f, 0.0f), glm::vec3(0.0f), glm::vec3(6.0f, 8.0f, 0.5f), constants::kWhiteWallMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(5.0f, 4.0f, 0.0f), glm::vec3(0.0f), glm::vec3(6.0f, 8.0f, 0.5f), constants::kWhiteWallMaterial},
    {common::scene::BuiltinMeshType::CUBE, glm::vec3(0.0f, 6.5f, 0.0f), glm::vec3(0.0f), glm::vec3(4.0f, 3.0f, 0.5f), constants::kWhiteWallMaterial},
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

} // namespace examples::post_processing_effects::exposure_and_adaptation::local_exposure
