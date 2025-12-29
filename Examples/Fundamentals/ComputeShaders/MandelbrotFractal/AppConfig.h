/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    10.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::compute_shaders::mandelbrot_fractal
{
namespace AppConstants
{
    // Shaders

    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto MainFragmentShaderFile = "AppConstants.MainFragmentShaderFile";
    constexpr auto MandelbrotComputeShaderFile = "AppConstants.MandelbrotComputeShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto MainFragmentShaderKey = "AppConstants.MainFragmentShaderKey";
    constexpr auto MandelbrotComputeShaderKey = "AppConstants.MandelbrotComputeShaderKey";

    // Resources
    constexpr auto MainVertexBuffer = "AppConstants.MainVertexBuffer";
    constexpr auto MainIndexBuffer = "AppConstants.MainIndexBuffer";
    constexpr auto MandelbrotStorageImage = "AppConstants.MandelbrotStorageImage";
    constexpr auto MandelbrotStorageImageView = "AppConstants.MandelbrotStorageImageView";
    constexpr auto MainSampler = "AppConstants.MainSampler";
    constexpr auto MainDescSetLayout = "AppConstants.MainDescSetLayout";
    constexpr auto ComputeDescSetLayout = "AppConstants.ComputeDescSetLayout";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto LoopTime = "AppSettings.LoopTime";
    constexpr auto ZoomSpeed = "AppSettings.ZoomSpeed";
} // namespace AppSettings
} // namespace examples::fundamentals::compute_shaders::mandelbrot_fractal
