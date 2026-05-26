/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    26.05.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::physically_based_rendering::ibl_and_reflections::equirectangular_hdr_skybox
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "pbr.vert.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "pbr.frag.spv";
    inline constexpr auto kSkyboxVertexShaderFile = "skybox.vert.spv";
    inline constexpr auto kConvertToCubemapFragmentShaderFile = "convert_to_cubemap.frag.spv";
    inline constexpr auto kSkyboxFragmentShaderFile = "skybox.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kSceneObjectsFragmentShaderKey = "fragScene";
    inline constexpr auto kSkyboxVertexShaderKey = "vertSkybox";
    inline constexpr auto kConvertToCubemapFragmentShaderKey = "fragConvertToCubemap";
    inline constexpr auto kSkyboxFragmentShaderKey = "fragSkybox";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";
    inline constexpr auto kEnvironmentCubemapImage = "environmentCubemapImage";
    inline constexpr auto kEnvironmentCubemapImageView = "environmentCubemapImageView";
    inline constexpr auto kEnvironmentCubemapImageViewRight = "environmentCubemapImageViewRight";
    inline constexpr auto kEnvironmentCubemapImageViewLeft = "environmentCubemapImageViewLeft";
    inline constexpr auto kEnvironmentCubemapImageViewTop = "environmentCubemapImageViewTop";
    inline constexpr auto kEnvironmentCubemapImageViewBottom = "environmentCubemapImageViewBottom";
    inline constexpr auto kEnvironmentCubemapImageViewBack = "environmentCubemapImageViewBack";
    inline constexpr auto kEnvironmentCubemapImageViewFront = "environmentCubemapImageViewFront";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kSkyboxSampler = "skyboxSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kConvertToCubemapDescSet = "convertToCubemapDescSet";
    inline constexpr auto kSkyboxDescSet = "skyboxDescSet";
    inline constexpr auto kSkyboxDescSetLayout = "skyboxDescSetLayout";

    // Textures
    inline constexpr auto kHdrEnvironmentTexturePath = "Textures/sundowner_deck_2k.hdr";
    inline constexpr auto kHdrEnvironmentTexture = "hdrEnvironmentTexture";

    inline constexpr auto kMetalDamagedAlbedoTexturePath = "Textures/Metal_Damaged_001_basecolor.jpg";
    inline constexpr auto kMetalDamagedAlbedoTexture = "metalDamagedAlbedoTexture";
    inline constexpr auto kMetalDamagedRoughnessTexturePath = "Textures/Metal_Damaged_001_roughness.jpg";
    inline constexpr auto kMetalDamagedRoughnessTexture = "metalDamagedRoughnessTexture";
    inline constexpr auto kMetalDamagedMetallicTexturePath = "Textures/Metal_Damaged_001_metallic.jpg";
    inline constexpr auto kMetalDamagedMetallicTexture = "metalDamagedMetallicTexture";
    inline constexpr auto kMetalDamagedNormalTexturePath = "Textures/Metal_Damaged_001_normal.jpg";
    inline constexpr auto kMetalDamagedNormalTexture = "metalDamagedNormalTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kSphereObject = "sphere";
    inline constexpr auto kSkyboxCubeObject = "skyboxCube";

    // Object Groups
    inline constexpr auto kSkyboxObjectGroup = "skyboxObjectGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraZoomSpeed = "AppSettings.CameraZoomSpeed";
} // namespace AppSettings
} // namespace examples::physically_based_rendering::ibl_and_reflections::equirectangular_hdr_skybox
