/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    25.01.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::environment_mapping::dynamic_cubemap_reflections
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "blinn_phong_directional.vert.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "blinn_phong_directional.frag.spv";
    inline constexpr auto kSkyboxVertexShaderFile = "skybox.vert.spv";
    inline constexpr auto kSkyboxFragmentShaderFile = "skybox.frag.spv";
    inline constexpr auto kReflectionFragmentShaderFile = "reflection.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kSceneObjectsFragmentShaderKey = "fragScene";
    inline constexpr auto kSkyboxVertexShaderKey = "vertSkybox";
    inline constexpr auto kSkyboxFragmentShaderKey = "fragSkybox";
    inline constexpr auto kReflectionFragmentShaderKey = "fragReflection";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";
    inline constexpr auto kReflectionDepthImage = "reflectionDepthImage";
    inline constexpr auto kReflectionDepthImageView = "reflectionDepthImageView";
    inline constexpr auto kReflectionCubemapImage = "reflectionCubemapImage";
    inline constexpr auto kReflectionCubemapImageView = "reflectionCubemapImageView";
    inline constexpr auto kReflectionCubemapImageViewRight = "reflectionCubemapImageViewRight";
    inline constexpr auto kReflectionCubemapImageViewLeft = "reflectionCubemapImageViewLeft";
    inline constexpr auto kReflectionCubemapImageViewTop = "reflectionCubemapImageViewTop";
    inline constexpr auto kReflectionCubemapImageViewBottom = "reflectionCubemapImageViewBottom";
    inline constexpr auto kReflectionCubemapImageViewBack = "reflectionCubemapImageViewBack";
    inline constexpr auto kReflectionCubemapImageViewFront = "reflectionCubemapImageViewFront";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kSkyboxSampler = "skyboxSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kSkyboxDescSet = "skyboxDescSet";
    inline constexpr auto kSkyboxDescSetLayout = "skyboxDescSetLayout";

    // Textures
    inline constexpr auto kWallStoneTexturePath = "Textures/Wall_Stone_034_basecolor.png";
    inline constexpr auto kWallStoneTexture = "wallStoneTexture";
    inline constexpr auto kWallStoneNormalTexturePath = "Textures/Wall_Stone_034_normal.png";
    inline constexpr auto kWallStoneNormalTexture = "wallStoneNormalTexture";
    inline constexpr auto kCubemapRightTexturePath = "Textures/calm_sea_cubemap/calm_sea_right.jpg";
    inline constexpr auto kCubemapLeftTexturePath = "Textures/calm_sea_cubemap/calm_sea_left.jpg";
    inline constexpr auto kCubemapTopTexturePath = "Textures/calm_sea_cubemap/calm_sea_up.jpg";
    inline constexpr auto kCubemapBottomTexturePath = "Textures/calm_sea_cubemap/calm_sea_down.jpg";
    inline constexpr auto kCubemapBackTexturePath = "Textures/calm_sea_cubemap/calm_sea_back.jpg";
    inline constexpr auto kCubemapFrontTexturePath = "Textures/calm_sea_cubemap/calm_sea_front.jpg";
    inline constexpr auto kCubemapTexture = "cubemapTexture";

    // Materials
    inline constexpr auto kDefaultMaterial = "defaultMaterial";
    inline constexpr auto kReflectiveMaterial = "reflectiveMaterial";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kCubeObject = "cube";
    inline constexpr auto kSphereObject = "sphere";
    inline constexpr auto kPlaneObject = "plane";
    inline constexpr auto kSkyboxCubeObject = "skyboxCube";

    // Object Groups
    inline constexpr auto kSkyboxObjectGroup = "skyboxObjectGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto CubemapResolution = "AppSettings.CubemapResolution";
} // namespace AppSettings
} // namespace examples::real_time_lighting::environment_mapping::dynamic_cubemap_reflections
