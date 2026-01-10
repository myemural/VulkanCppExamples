/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    04.01.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::surface_detailing::basic_displacement
{
namespace AppConstants
{
    // Shaders
    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto SceneObjectsFragmentShaderFile = "AppConstants.SceneFragmentShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto SceneObjectsFragmentShaderKey = "AppConstants.SceneFragmentShaderKey";

    // Resources
    constexpr auto LightUniformBuffer = "AppConstants.LightUniformBuffer";
    constexpr auto DepthImage = "AppConstants.DepthImage";
    constexpr auto DepthImageView = "AppConstants.DepthImageView";
    constexpr auto MainSampler = "AppConstants.MainSampler";
    constexpr auto MainDescSet = "AppConstants.MainDescSet";
    constexpr auto MainDescSetLayout = "AppConstants.MainDescSetLayout";
    constexpr auto MetalPatternTexturePath = "AppConstants.MetalPatternTexturePath";
    constexpr auto MetalPatternTexture = "AppConstants.MetalPatternTexture";
    constexpr auto MetalPatternNormalTexturePath = "AppConstants.MetalPatternNormalTexturePath";
    constexpr auto MetalPatternNormalTexture = "AppConstants.MetalPatternNormalTexture";
    constexpr auto MetalPatternHeightTexturePath = "AppConstants.MetalPatternHeightTexturePath";
    constexpr auto MetalPatternHeightTexture = "AppConstants.MetalPatternHeightTexture";
    constexpr auto DefaultMaterial = "AppConstants.DefaultMaterial";

    // Objects
    constexpr auto CameraObject = "AppConstants.CameraObject";
    constexpr auto SphereObject1 = "AppConstants.SphereObject1";
    constexpr auto SphereObject2 = "AppConstants.SphereObject2";
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
} // namespace examples::real_time_lighting::surface_detailing::basic_displacement
