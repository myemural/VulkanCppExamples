/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    16.05.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::physically_based_rendering::textured_pbr::metallic_map_pbr
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

    inline constexpr auto kMetalDamagedAlbedoTexturePath = "Textures/Metal_Damaged_001_basecolor.jpg";
    inline constexpr auto kMetalDamagedAlbedoTexture = "metalDamagedAlbedoTexture";
    inline constexpr auto kMetalDamagedRoughnessTexturePath = "Textures/Metal_Damaged_001_roughness.jpg";
    inline constexpr auto kMetalDamagedRoughnessTexture = "metalDamagedRoughnessTexture";
    inline constexpr auto kMetalDamagedMetallicTexturePath = "Textures/Metal_Damaged_001_metallic.jpg";
    inline constexpr auto kMetalDamagedMetallicTexture = "metalDamagedMetallicTexture";

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
    constexpr auto LightDirection = "AppSettings.LightDirection";
    constexpr auto LightColor = "AppSettings.LightColor";
} // namespace AppSettings
} // namespace examples::physically_based_rendering::textured_pbr::metallic_map_pbr
