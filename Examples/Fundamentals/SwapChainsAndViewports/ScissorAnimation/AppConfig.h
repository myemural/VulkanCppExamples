/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    8.10.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::swap_chains_and_viewports::scissor_animation
{
namespace AppConstants
{
    constexpr auto MaxFramesInFlight = "AppConstants.MaxFramesInFlight";

    // Shaders
    constexpr auto BaseShaderType = "AppConstants.BaseShaderType";
    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto MainFragmentShaderFile = "AppConstants.MainFragmentShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto MainFragmentShaderKey = "AppConstants.MainFragmentShaderKey";

    // Resources
    constexpr auto MainVertexBuffer = "AppConstants.MainVertexBuffer";
    constexpr auto MainIndexBuffer = "AppConstants.MainIndexBuffer";
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
} // namespace AppSettings
} // namespace examples::fundamentals::swap_chains_and_viewports::scissor_animation
