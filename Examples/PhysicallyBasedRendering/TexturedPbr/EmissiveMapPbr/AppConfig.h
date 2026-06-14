/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    19.05.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::physically_based_rendering::textured_pbr::emissive_map_pbr
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "pbr.vert.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "pbr.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kSceneObjectsFragmentShaderKey = "fragScene";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";

    // Textures
    inline constexpr auto kWoodFloorAlbedoTexturePath = "Textures/Wood_Floor_015_basecolor.png";
    inline constexpr auto kWoodFloorAlbedoTexture = "woodFloorAlbedoTexture";
    inline constexpr auto kWoodFloorRoughnessTexturePath = "Textures/Wood_Floor_015_roughness.png";
    inline constexpr auto kWoodFloorRoughnessTexture = "woodFloorRoughnessTexture";
    inline constexpr auto kWoodFloorNormalTexturePath = "Textures/Wood_Floor_015_normal.png";
    inline constexpr auto kWoodFloorNormalTexture = "woodFloorNormalTexture";
    inline constexpr auto kWoodFloorAoTexturePath = "Textures/Wood_Floor_015_ambientOcclusion.png";
    inline constexpr auto kWoodFloorAoTexture = "woodFloorAmbientOcclusionTexture";

    inline constexpr auto kCeilingAlbedoTexturePath = "Textures/Ceiling_Gypsum_001_Base_Color.jpg";
    inline constexpr auto kCeilingAlbedoTexture = "ceilingAlbedoTexture";
    inline constexpr auto kCeilingRoughnessTexturePath = "Textures/Ceiling_Gypsum_001_roughness.jpg";
    inline constexpr auto kCeilingRoughnessTexture = "ceilingRoughnessTexture";
    inline constexpr auto kCeilingMetallicTexturePath = "Textures/Ceiling_Gypsum_001_metallic.jpg";
    inline constexpr auto kCeilingMetallicTexture = "ceilingMetallicTexture";
    inline constexpr auto kCeilingNormalTexturePath = "Textures/Ceiling_Gypsum_001_normal.jpg";
    inline constexpr auto kCeilingNormalTexture = "ceilingNormalTexture";
    inline constexpr auto kCeilingAoTexturePath = "Textures/Ceiling_Gypsum_001_ambientOcclusion.jpg";
    inline constexpr auto kCeilingAoTexture = "ceilingAmbientOcclusionTexture";
    inline constexpr auto kCeilingEmissiveTexturePath = "Textures/Ceiling_Gypsum_001_emissive.jpg";
    inline constexpr auto kCeilingEmissiveTexture = "ceilingEmissiveTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kCubeObject1 = "cube1";
    inline constexpr auto kCubeObject2 = "cube2";
    inline constexpr auto kSphereObject1 = "sphere1";
    inline constexpr auto kSphereObject2 = "sphere2";
    inline constexpr auto kConeObject = "cone";
    inline constexpr auto kFloorObject = "floor";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::physically_based_rendering::textured_pbr::emissive_map_pbr
