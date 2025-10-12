/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    29.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::pipelines_and_passes::offscreen_rendering
{
namespace AppConstants
{
    constexpr auto MaxFramesInFlight = "AppConstants.MaxFramesInFlight";

    // Shaders
    constexpr auto BaseShaderType = "AppConstants.BaseShaderType";
    constexpr auto QuadVertexShaderFile = "AppConstants.QuadVertexShaderFile";
    constexpr auto SceneVertexShaderFile = "AppConstants.SceneVertexShaderFile";
    constexpr auto QuadFragmentShaderFile = "AppConstants.QuadFragmentShaderFile";
    constexpr auto SceneFragmentShaderFile = "AppConstants.SceneFragmentShaderFile";
    constexpr auto QuadVertexShaderKey = "AppConstants.QuadVertexShaderKey";
    constexpr auto SceneVertexShaderKey = "AppConstants.SceneVertexShaderKey";
    constexpr auto QuadFragmentShaderKey = "AppConstants.QuadFragmentShaderKey";
    constexpr auto SceneFragmentShaderKey = "AppConstants.SceneFragmentShaderKey";

    // Resources
    constexpr auto CubeVertexBuffer = "AppConstants.CubeVertexBuffer";
    constexpr auto CubeIndexBuffer = "AppConstants.CubeIndexBuffer";
    constexpr auto PlaneVertexBuffer = "AppConstants.PlaneVertexBuffer";
    constexpr auto PlaneIndexBuffer = "AppConstants.PlaneIndexBuffer";
    constexpr auto CrateImage = "AppConstants.CrateImage";
    constexpr auto CrateImageView = "AppConstants.CrateImageView";
    constexpr auto DepthImage = "AppConstants.DepthImage";
    constexpr auto DepthImageView = "AppConstants.DepthImageView";
    constexpr auto OffscreenImage = "AppConstants.OffscreenImage";
    constexpr auto OffscreenImageView = "AppConstants.OffscreenImageView";
    constexpr auto MainSampler = "AppConstants.MainSampler";
    constexpr auto SceneDescSetLayout = "AppConstants.SceneDescSetLayout";
    constexpr auto QuadDescSetLayout = "AppConstants.QuadDescSetLayout";
    constexpr auto CrateTexturePath = "AppConstants.CrateTexturePath";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::fundamentals::pipelines_and_passes::offscreen_rendering
