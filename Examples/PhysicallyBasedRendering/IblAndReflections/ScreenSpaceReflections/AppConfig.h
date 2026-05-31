/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    31.05.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::physically_based_rendering::ibl_and_reflections::screen_space_reflections
{
namespace constants
{
    // Shaders
    inline constexpr auto kGeometryVertexShaderFile = "geometry_pass.vert.spv";
    inline constexpr auto kGeometryFragmentShaderFile = "geometry_pass.frag.spv";
    inline constexpr auto kLightingVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kLightingFragmentShaderFile = "lighting_pass.frag.spv";
    inline constexpr auto kSsrFragmentShaderFile = "ssr_pass.frag.spv";
    inline constexpr auto kGeometryVertexShaderKey = "vertGeometry";
    inline constexpr auto kGeometryFragmentShaderKey = "fragGeometry";
    inline constexpr auto kLightingVertexShaderKey = "vertLighting";
    inline constexpr auto kLightingFragmentShaderKey = "fragLighting";
    inline constexpr auto kSsrFragmentShaderKey = "fragSsr";

    // Buffers
    inline constexpr auto kLightStorageBuffer = "lightStorageBuffer";

    // Images and Image Views
    inline constexpr auto kPositionImage = "positionImage";
    inline constexpr auto kPositionImageView = "positionImageView";
    inline constexpr auto kAlbedoImage = "albedoImage";
    inline constexpr auto kAlbedoImageView = "albedoImageView";
    inline constexpr auto kMetallicRoughnessImage = "metallicRoughnessImage";
    inline constexpr auto kMetallicRoughnessImageView = "metallicRoughnessImageView";
    inline constexpr auto kNormalImage = "normalImage";
    inline constexpr auto kNormalImageView = "normalImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";
    inline constexpr auto kSceneColorImage = "sceneColorImage";
    inline constexpr auto kSceneColorImageView = "sceneColorImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kLightDescSet = "lightDescSet";
    inline constexpr auto kLightDescSetLayout = "lightDescSetLayout";
    inline constexpr auto kSsrDescSet = "ssrDescSet";
    inline constexpr auto kSsrDescSetLayout = "ssrDescSetLayout";

    // Textures
    inline constexpr auto kWoodFloorAlbedoTexturePath = "Textures/Wood_Floor_015_basecolor.png";
    inline constexpr auto kWoodFloorAlbedoTexture = "woodFloorAlbedoTexture";
    inline constexpr auto kWoodFloorRoughnessTexturePath = "Textures/Wood_Floor_015_roughness.png";
    inline constexpr auto kWoodFloorRoughnessTexture = "woodFloorRoughnessTexture";
    inline constexpr auto kWoodFloorNormalTexturePath = "Textures/Wood_Floor_015_normal.png";
    inline constexpr auto kWoodFloorNormalTexture = "woodFloorNormalTexture";

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
    constexpr auto SsrMaxDistance = "AppSettings.SsrMaxDistance";
    constexpr auto SsrThickness = "AppSettings.SsrThickness";
    constexpr auto SsrMaxSteps = "AppSettings.SsrMaxSteps";
    constexpr auto SsrBinarySearchSteps = "AppSettings.SsrBinarySearchSteps";
} // namespace AppSettings
} // namespace examples::physically_based_rendering::ibl_and_reflections::screen_space_reflections
