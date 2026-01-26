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
namespace AppConstants
{
    // Shaders
    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto SceneObjectsFragmentShaderFile = "AppConstants.SceneFragmentShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto SceneObjectsFragmentShaderKey = "AppConstants.SceneFragmentShaderKey";
    constexpr auto SkyboxVertexShaderFile = "AppConstants.SkyboxVertexShaderFile";
    constexpr auto SkyboxVertexShaderKey = "AppConstants.SkyboxVertexShaderKey";
    constexpr auto SkyboxFragmentShaderFile = "AppConstants.SkyboxFragmentShaderFile";
    constexpr auto SkyboxFragmentShaderKey = "AppConstants.SkyboxFragmentShaderKey";
    constexpr auto ReflectionFragmentShaderFile = "AppConstants.ReflectionFragmentShaderFile";
    constexpr auto ReflectionFragmentShaderKey = "AppConstants.ReflectionFragmentShaderKey";

    // Buffers
    constexpr auto LightUniformBuffer = "AppConstants.LightUniformBuffer";

    // Images and image views (for generated images)
    constexpr auto DepthImage = "AppConstants.DepthImage";
    constexpr auto DepthImageView = "AppConstants.DepthImageView";
    constexpr auto ReflectionDepthImage = "AppConstants.ReflectionDepthImage";
    constexpr auto ReflectionDepthImageView = "AppConstants.ReflectionDepthImageView";
    constexpr auto ReflectionCubemapImage = "AppConstants.ReflectionCubemapImage";
    constexpr auto ReflectionCubemapImageView = "AppConstants.ReflectionCubemapImageView";
    constexpr auto ReflectionCubemapImageViewRight = "AppConstants.ReflectionCubemapImageViewRight";
    constexpr auto ReflectionCubemapImageViewLeft = "AppConstants.ReflectionCubemapImageViewLeft";
    constexpr auto ReflectionCubemapImageViewTop = "AppConstants.ReflectionCubemapImageViewTop";
    constexpr auto ReflectionCubemapImageViewBottom = "AppConstants.ReflectionCubemapImageViewBottom";
    constexpr auto ReflectionCubemapImageViewBack = "AppConstants.ReflectionCubemapImageViewBack";
    constexpr auto ReflectionCubemapImageViewFront = "AppConstants.ReflectionCubemapImageViewFront";

    // Samplers
    constexpr auto MainSampler = "AppConstants.MainSampler";
    constexpr auto SkyboxSampler = "AppConstants.SkyboxSampler";

    // Descriptor sets and layouts
    constexpr auto MainDescSet = "AppConstants.MainDescSet";
    constexpr auto SkyboxDescSet = "AppConstants.SkyboxDescSet";
    constexpr auto MainDescSetLayout = "AppConstants.MainDescSetLayout";
    constexpr auto SkyboxDescSetLayout = "AppConstants.SkyboxDescSetLayout";

    // Textures
    constexpr auto WallStoneTexturePath = "AppConstants.WallStoneTexturePath";
    constexpr auto WallStoneTexture = "AppConstants.WallStoneTexture";
    constexpr auto WallStoneNormalTexturePath = "AppConstants.WallStoneNormalTexturePath";
    constexpr auto WallStoneNormalTexture = "AppConstants.WallStoneNormalTexture";
    constexpr auto CubemapTexture = "AppConstants.CubemapTexture";
    constexpr auto CubemapRightTexturePath = "AppConstants.CubemapRightTexturePath";
    constexpr auto CubemapLeftTexturePath = "AppConstants.CubemapLeftTexturePath";
    constexpr auto CubemapTopTexturePath = "AppConstants.CubemapTopTexturePath";
    constexpr auto CubemapBottomTexturePath = "AppConstants.CubemapBottomTexturePath";
    constexpr auto CubemapBackTexturePath = "AppConstants.CubemapBackTexturePath";
    constexpr auto CubemapFrontTexturePath = "AppConstants.CubemapFrontTexturePath";

    // Materials
    constexpr auto DefaultMaterial = "AppConstants.DefaultMaterial";
    constexpr auto ReflectiveMaterial = "AppConstants.ReflectiveMaterial";

    // Objects
    constexpr auto CameraObject = "AppConstants.CameraObject";
    constexpr auto CubeObject = "AppConstants.CubeObject";
    constexpr auto SphereObject = "AppConstants.SphereObject";
    constexpr auto PlaneObject = "AppConstants.PlaneObject";
    constexpr auto SkyboxCubeObject = "AppConstants.SkyboxCubeObject";

    // Object groups
    constexpr auto SkyboxObjectGroup = "AppConstants.SkyboxObjectGroup";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto LightDirection = "AppSettings.LightDirection";
    constexpr auto LightColor = "AppSettings.LightColor";
    constexpr auto AmbientStrength = "AppSettings.AmbientStrength";
    constexpr auto SpecularStrength = "AppSettings.SpecularStrength";
    constexpr auto Shininess = "AppSettings.Shininess";
} // namespace AppSettings
} // namespace examples::real_time_lighting::environment_mapping::dynamic_cubemap_reflections
