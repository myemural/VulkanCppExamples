/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    16.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "AppCommonConfig.h"

namespace examples::fundamentals::drawing_3d::depth_testing_operations
{
namespace AppConstants
{
    constexpr auto MaxFramesInFlight = "AppConstants.MaxFramesInFlight";
    constexpr auto BaseShaderType = "AppConstants.BaseShaderType";
    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto MainFragmentShaderFile = "AppConstants.MainFragmentShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto MainFragmentShaderKey = "AppConstants.MainFragmentShaderKey";

    // Resources
    constexpr auto CubeVertexBuffer = "AppConstants.CubeVertexBuffer";
    constexpr auto CubeIndexBuffer = "AppConstants.CubeIndexBuffer";
    constexpr auto PlaneVertexBuffer = "AppConstants.PlaneVertexBuffer";
    constexpr auto PlaneIndexBuffer = "AppConstants.PlaneIndexBuffer";
    constexpr auto ImageStagingBuffer = "AppConstants.ImageStagingBuffer";
    constexpr auto CrateImage = "AppConstants.CrateImage";
    constexpr auto CrateImageView = "AppConstants.CrateImageView";
    constexpr auto DepthImage = "AppConstants.DepthImage";
    constexpr auto DepthImageView = "AppConstants.DepthImageView";
    constexpr auto MainSampler = "AppConstants.MainSampler";
    constexpr auto MainDescSetLayout = "AppConstants.MainDescSetLayout";
    constexpr auto CrateTexturePath = "AppConstants.CrateTexturePath";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto DepthTestEnabled = "AppSettings.DepthTestEnabled";
    constexpr auto DepthWriteEnabled = "AppSettings.DepthWriteEnabled";
    constexpr auto DepthCompareOp = "AppSettings.DepthCompareOp";
} // namespace AppSettings
} // namespace examples::fundamentals::drawing_3d::depth_testing_operations
