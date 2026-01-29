/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
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
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "passthrough_position_uv.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "texture_sampler.frag.spv";
    inline constexpr auto kMandelbrotComputeShaderFile = "mandelbrot_generator.comp.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";
    inline constexpr auto kMandelbrotComputeShaderKey = "compMandelbrot";

    // Buffers
    inline constexpr auto kMainVertexBuffer = "mainVertexBuffer";
    inline constexpr auto kMainIndexBuffer = "mainIndexBuffer";

    // Images and Image Views
    inline constexpr auto kMandelbrotStorageImage = "mandelbrotStorageImage";
    inline constexpr auto kMandelbrotStorageImageView = "mandelbrotStorageImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kComputeDescSetLayout = "computeDescSetLayout";
} // namespace constants

namespace AppSettings
{
    constexpr auto LoopTime = "AppSettings.LoopTime";
    constexpr auto ZoomSpeed = "AppSettings.ZoomSpeed";
} // namespace AppSettings
} // namespace examples::fundamentals::compute_shaders::mandelbrot_fractal
