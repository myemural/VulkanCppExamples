/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    9.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::compute_shaders::checkerboard_generator
{
namespace AppConstants
{
    constexpr auto MaxFramesInFlight = "AppConstants.MaxFramesInFlight";

    // Shaders
    constexpr auto BaseShaderType = "AppConstants.BaseShaderType";
    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto MainFragmentShaderFile = "AppConstants.MainFragmentShaderFile";
    constexpr auto CheckerboardComputeShaderFile = "AppConstants.CheckerboardComputeShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto MainFragmentShaderKey = "AppConstants.MainFragmentShaderKey";
    constexpr auto CheckerboardComputeShaderKey = "AppConstants.CheckerboardComputeShaderKey";

    // Resources
    constexpr auto CubeVertexBuffer = "AppConstants.CubeVertexBuffer";
    constexpr auto CubeIndexBuffer = "AppConstants.CubeIndexBuffer";
    constexpr auto SphereVertexBuffer = "AppConstants.SphereVertexBuffer";
    constexpr auto SphereIndexBuffer = "AppConstants.SphereIndexBuffer";
    constexpr auto CheckerboardStorageImage = "AppConstants.CheckerboardStorageImage";
    constexpr auto CheckerboardStorageImageView = "AppConstants.CheckerboardStorageImageView";
    constexpr auto CrateImage = "AppConstants.CrateImage";
    constexpr auto CrateImageView = "AppConstants.CrateImageView";
    constexpr auto DepthImage = "AppConstants.DepthImage";
    constexpr auto DepthImageView = "AppConstants.DepthImageView";
    constexpr auto MainSampler = "AppConstants.MainSampler";
    constexpr auto CubeDescSet = "AppConstants.CubeDescSet";
    constexpr auto SphereDescSet = "AppConstants.SphereDescSet";
    constexpr auto MainDescSetLayout = "AppConstants.MainDescSetLayout";
    constexpr auto ComputeDescSetLayout = "AppConstants.ComputeDescSetLayout";
    constexpr auto CrateTexturePath = "AppConstants.CrateTexturePath";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto CheckerboardTextureSize = "AppSettings.CheckerboardTextureSize";
    constexpr auto TileSize = "AppSettings.TileSize";
} // namespace AppSettings
} // namespace examples::fundamentals::compute_shaders::checkerboard_generator
