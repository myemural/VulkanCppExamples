/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    24.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::tessellation_shaders::cubic_bezier_curve_tessellation
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "bezier_curve.vert.spv";
    inline constexpr auto kMainTessControlShaderFile = "bezier_curve.tesc.spv";
    inline constexpr auto kMainTessEvaluationShaderFile = "bezier_curve.tese.spv";
    inline constexpr auto kMainFragmentShaderFile = "bezier_curve.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainTessControlShaderKey = "tescMain";
    inline constexpr auto kMainTessEvaluationShaderKey = "teseMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";

    // Buffers
    inline constexpr auto kControlPointsBuffer = "controlPointsBuffer";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto TessellationLevel = "AppSettings.TessellationLevel";
    constexpr auto ControlPointSize = "AppSettings.ControlPointSize";
    constexpr auto ControlPointColor = "AppSettings.ControlPointColor";
    constexpr auto CurveColor = "AppSettings.CurveColor";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::tessellation_shaders::cubic_bezier_curve_tessellation
