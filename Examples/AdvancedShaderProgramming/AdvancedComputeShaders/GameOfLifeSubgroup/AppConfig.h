/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    15.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::advanced_compute_shaders::game_of_life_subgroup
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "texture_sampler.frag.spv";
    inline constexpr auto kGameOfLifeComputeShaderFile = "game_of_life.comp.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";
    inline constexpr auto kGameOfLifeComputeShaderKey = "compGameOfLife";

    // Images and Image Views
    inline constexpr auto kScreenStorageImage = "screenStorageImage";
    inline constexpr auto kScreenStorageImageView = "screenStorageImageView";
    inline constexpr auto kCellStateImageA = "cellStateImageA";
    inline constexpr auto kCellStateImageAView = "cellStateImageAView";
    inline constexpr auto kCellStateImageB = "cellStateImageB";
    inline constexpr auto kCellStateImageBView = "cellStateImageBView";

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
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::game_of_life_subgroup
