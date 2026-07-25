/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    25.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::tessellation_shaders::bezier_surface_tessellation
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "bezier_surface.vert.spv";
    inline constexpr auto kMainTessControlShaderFile = "bezier_surface.tesc.spv";
    inline constexpr auto kMainTessEvaluationShaderFile = "bezier_surface.tese.spv";
    inline constexpr auto kMainFragmentShaderFile = "bezier_surface.frag.spv";
    inline constexpr auto kControlPointsVertexShaderFile = "control_points.vert.spv";
    inline constexpr auto kControlPointsFragmentShaderFile = "control_points.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainTessControlShaderKey = "tescMain";
    inline constexpr auto kMainTessEvaluationShaderKey = "teseMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";
    inline constexpr auto kControlPointsVertexShaderKey = "vertControlPoints";
    inline constexpr auto kControlPointsFragmentShaderKey = "fragControlPoints";

    // Buffers
    inline constexpr auto kControlPointsBuffer = "controlPointsBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto CameraMoveSensitivity = "AppSettings.CameraMoveSensitivity";
    constexpr auto CameraZoomSpeed = "AppSettings.CameraZoomSpeed";
    constexpr auto PolygonMode = "AppSettings.PolygonMode";
    constexpr auto TessellationLevel = "AppSettings.TessellationLevel";
    constexpr auto ControlPointSize = "AppSettings.ControlPointSize";
    constexpr auto ControlPointColor = "AppSettings.ControlPointColor";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::tessellation_shaders::bezier_surface_tessellation
