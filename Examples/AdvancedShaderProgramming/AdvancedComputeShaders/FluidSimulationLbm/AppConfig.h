/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    28.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_lbm
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "texture_sampler.frag.spv";
    inline constexpr auto kCollideStreamComputeShaderFile = "collide_stream.comp.spv";
    inline constexpr auto kAdvectDyeComputeShaderFile = "advect_dye.comp.spv";
    inline constexpr auto kPaintObstacleComputeShaderFile = "paint_obstacle.comp.spv";
    inline constexpr auto kColorizeComputeShaderFile = "colorize.comp.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";
    inline constexpr auto kCollideStreamComputeShaderKey = "compCollideStream";
    inline constexpr auto kAdvectDyeComputeShaderKey = "compAdvectDye";
    inline constexpr auto kPaintObstacleComputeShaderKey = "compPaintObstacle";
    inline constexpr auto kColorizeComputeShaderKey = "compColorize";

    // Buffers
    inline constexpr auto kDistributionBufferA = "distributionBufferA";
    inline constexpr auto kDistributionBufferB = "distributionBufferB";
    inline constexpr auto kDyeBufferA = "dyeBufferA";
    inline constexpr auto kDyeBufferB = "dyeBufferB";
    inline constexpr auto kObstacleBuffer = "obstacleBuffer";

    // Images and Image Views
    inline constexpr auto kScreenStorageImage = "screenStorageImage";
    inline constexpr auto kScreenStorageImageView = "screenStorageImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kComputeDescSetAToB = "computeDescSetAToB";
    inline constexpr auto kComputeDescSetBToA = "computeDescSetBToA";
    inline constexpr auto kComputeDescSetLayout = "computeDescSetLayout";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_lbm
