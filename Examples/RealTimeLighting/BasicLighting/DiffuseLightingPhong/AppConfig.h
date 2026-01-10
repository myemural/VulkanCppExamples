/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    13.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::basic_lighting::diffuse_lighting_phong
{
namespace AppConstants
{
    // Shaders
    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto SceneObjectsFragmentShaderFile = "AppConstants.SceneFragmentShaderFile";
    constexpr auto LightObjectsFragmentShaderFile = "AppConstants.LightFragmentShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto SceneObjectsFragmentShaderKey = "AppConstants.SceneFragmentShaderKey";
    constexpr auto LightObjectsFragmentShaderKey = "AppConstants.LightFragmentShaderKey";

    // Resources
    constexpr auto LightUniformBuffer = "AppConstants.LightUniformBuffer";
    constexpr auto DepthImage = "AppConstants.DepthImage";
    constexpr auto DepthImageView = "AppConstants.DepthImageView";
    constexpr auto MainDescSet = "AppConstants.MainDescSet";
    constexpr auto MainDescSetLayout = "AppConstants.MainDescSetLayout";
    constexpr auto DefaultMaterial = "AppConstants.DefaultMaterial";

    // Objects
    constexpr auto CameraObject = "AppConstants.CameraObject";
    constexpr auto CubeObject = "AppConstants.CubeObject";
    constexpr auto SphereObject = "AppConstants.SphereObject";
    constexpr auto ConeObject = "AppConstants.ConeObject";
    constexpr auto CylinderObject = "AppConstants.CylinderObject";
    constexpr auto PlaneObject = "AppConstants.PlaneObject";
    constexpr auto LightObject = "AppConstants.LightObject";

    // Groups
    constexpr auto LightGroup = "AppConstants.LightGroup";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto LightColor = "AppSettings.LightColor";
    constexpr auto AmbientStrength = "AppSettings.AmbientStrength";
} // namespace AppSettings
} // namespace examples::real_time_lighting::basic_lighting::diffuse_lighting_phong
