/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    19.01.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::environment_mapping::fresnel_effect
{
namespace AppConstants
{
    // Shaders
    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto SceneObjectsFragmentShaderFile = "AppConstants.SceneFragmentShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto SceneObjectsFragmentShaderKey = "AppConstants.SceneFragmentShaderKey";
    constexpr auto SkyboxVertexShaderFile = "AppConstants.SkyboxVertexShaderFile";
    constexpr auto SkyboxFragmentShaderFile = "AppConstants.SkyboxFragmentShaderFile";
    constexpr auto SkyboxVertexShaderKey = "AppConstants.SkyboxVertexShaderKey";
    constexpr auto SkyboxFragmentShaderKey = "AppConstants.SkyboxFragmentShaderKey";

    // Resources
    constexpr auto LightUniformBuffer = "AppConstants.LightUniformBuffer";
    constexpr auto DepthImage = "AppConstants.DepthImage";
    constexpr auto DepthImageView = "AppConstants.DepthImageView";
    constexpr auto MainSampler = "AppConstants.MainSampler";
    constexpr auto SkyboxSampler = "AppConstants.SkyboxSampler";
    constexpr auto MainDescSet = "AppConstants.MainDescSet";
    constexpr auto SkyboxDescSet = "AppConstants.SkyboxDescSet";
    constexpr auto MainDescSetLayout = "AppConstants.MainDescSetLayout";
    constexpr auto SkyboxDescSetLayout = "AppConstants.SkyboxDescSetLayout";
    constexpr auto FloorTexturePath = "AppConstants.FloorTexturePath";
    constexpr auto FloorTexture = "AppConstants.FloorTexture";
    constexpr auto FloorNormalTexturePath = "AppConstants.FloorNormalTexturePath";
    constexpr auto FloorNormalTexture = "AppConstants.FloorNormalTexture";
    constexpr auto CubemapTexture = "AppConstants.CubemapTexture";
    constexpr auto CubemapRightTexturePath = "AppConstants.CubemapRightTexturePath";
    constexpr auto CubemapLeftTexturePath = "AppConstants.CubemapLeftTexturePath";
    constexpr auto CubemapTopTexturePath = "AppConstants.CubemapTopTexturePath";
    constexpr auto CubemapBottomTexturePath = "AppConstants.CubemapBottomTexturePath";
    constexpr auto CubemapBackTexturePath = "AppConstants.CubemapBackTexturePath";
    constexpr auto CubemapFrontTexturePath = "AppConstants.CubemapFrontTexturePath";
    constexpr auto DefaultMaterial = "AppConstants.DefaultMaterial";
    constexpr auto SkyboxObjectGroup = "AppConstants.SkyboxObjectGroup";

    // Objects
    constexpr auto CameraObject = "AppConstants.CameraObject";
    constexpr auto PlaneObject = "AppConstants.PlaneObject";
    constexpr auto SkyboxCubeObject = "AppConstants.SkyboxCubeObject";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraZoomSpeed = "AppSettings.CameraZoomSpeed";
    constexpr auto LightDirection = "AppSettings.LightDirection";
    constexpr auto LightColor = "AppSettings.LightColor";
    constexpr auto AmbientStrength = "AppSettings.AmbientStrength";
    constexpr auto SpecularStrength = "AppSettings.SpecularStrength";
    constexpr auto Shininess = "AppSettings.Shininess";
} // namespace AppSettings
} // namespace examples::real_time_lighting::environment_mapping::fresnel_effect
