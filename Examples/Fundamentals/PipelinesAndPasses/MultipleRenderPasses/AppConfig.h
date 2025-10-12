/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    28.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::pipelines_and_passes::multiple_render_passes
{
namespace AppConstants
{
    constexpr auto MaxFramesInFlight = "AppConstants.MaxFramesInFlight";

    // Shaders
    constexpr auto BaseShaderType = "AppConstants.BaseShaderType";
    constexpr auto BackgroundVertexShaderFile = "AppConstants.BackgroundVertexShaderFile";
    constexpr auto BackgroundFragmentShaderFile = "AppConstants.BackgroundFragmentShaderFile";
    constexpr auto CubeVertexShaderFile = "AppConstants.CubeVertexShaderFile";
    constexpr auto CubeFragmentShaderFile = "AppConstants.CubeFragmentShaderFile";
    constexpr auto BackgroundVertexShaderKey = "AppConstants.BackgroundVertexShaderKey";
    constexpr auto BackgroundFragmentShaderKey = "AppConstants.BackgroundFragmentShaderKey";
    constexpr auto CubeVertexShaderKey = "AppConstants.CubeVertexShaderKey";
    constexpr auto CubeFragmentShaderKey = "AppConstants.CubeFragmentShaderKey";

    // Resources
    constexpr auto CubeVertexBuffer = "AppConstants.CubeVertexBuffer";
    constexpr auto CubeIndexBuffer = "AppConstants.CubeIndexBuffer";
    constexpr auto PlaneVertexBuffer = "AppConstants.PlaneVertexBuffer";
    constexpr auto PlaneIndexBuffer = "AppConstants.PlaneIndexBuffer";
    constexpr auto TimeSpeedUniformBuffer = "AppConstants.TimeSpeedUniformBuffer";
    constexpr auto CrateImage = "AppConstants.CrateImage";
    constexpr auto CrateImageView = "AppConstants.CrateImageView";
    constexpr auto CloudImage = "AppConstants.CloudImage";
    constexpr auto CloudImageView = "AppConstants.CloudImageView";
    constexpr auto DepthImage = "AppConstants.DepthImage";
    constexpr auto DepthImageView = "AppConstants.DepthImageView";
    constexpr auto MainSampler = "AppConstants.MainSampler";
    constexpr auto BackgroundDescSetLayout = "AppConstants.BackgroundDescSetLayout";
    constexpr auto CubeDescSetLayout = "AppConstants.CubeDescSetLayout";
    constexpr auto CrateTexturePath = "AppConstants.CrateTexturePath";
    constexpr auto CloudTexturePath = "AppConstants.CloudTexturePath";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto CloudSpeed = "AppSettings.CloudSpeed";
} // namespace AppSettings
} // namespace examples::fundamentals::pipelines_and_passes::multiple_render_passes
