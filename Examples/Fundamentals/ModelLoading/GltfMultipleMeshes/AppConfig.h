/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    15.10.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::model_loading::gltf_multiple_meshes
{
namespace AppConstants
{
    // Shaders
    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto MainFragmentShaderFile = "AppConstants.MainFragmentShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto MainFragmentShaderKey = "AppConstants.MainFragmentShaderKey";

    // Resources
    constexpr auto MeshImage = "AppConstants.MeshImage";
    constexpr auto MeshImageView = "AppConstants.MeshImageView";
    constexpr auto DepthImage = "AppConstants.DepthImage";
    constexpr auto DepthImageView = "AppConstants.DepthImageView";
    constexpr auto MainSampler = "AppConstants.MainSampler";
    constexpr auto MainDescSetLayout = "AppConstants.MainDescSetLayout";
    constexpr auto LanternModelPath = "AppConstants.LanternModelPath";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::fundamentals::model_loading::gltf_multiple_meshes
