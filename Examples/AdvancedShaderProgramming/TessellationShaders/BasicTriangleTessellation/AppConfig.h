/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    20.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::tessellation_shaders::basic_triangle_tessellation
{
namespace constants
{
    // Drawing
    inline constexpr auto kVertexCount = 3U;

    // Shaders
    inline constexpr auto kMainVertexShaderFile = "triangle_tessellation.vert.spv";
    inline constexpr auto kMainTessControlShaderFile = "triangle_tessellation.tesc.spv";
    inline constexpr auto kMainTessEvaluationShaderFile = "triangle_tessellation.tese.spv";
    inline constexpr auto kMainFragmentShaderFile = "triangle_tessellation.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainTessControlShaderKey = "tescMain";
    inline constexpr auto kMainTessEvaluationShaderKey = "teseMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto LineWidth = "AppSettings.LineWidth";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::tessellation_shaders::basic_triangle_tessellation
