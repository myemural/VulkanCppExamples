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

namespace examples::advanced_shader_programming::tessellation_shaders::curved_pn_triangles_tessellation
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "curved_pn_triangles.vert.spv";
    inline constexpr auto kMainTessControlShaderFile = "curved_pn_triangles.tesc.spv";
    inline constexpr auto kMainTessEvaluationShaderFile = "curved_pn_triangles.tese.spv";
    inline constexpr auto kMainFragmentShaderFile = "curved_pn_triangles.frag.spv";
    inline constexpr auto kControlPointsVertexShaderFile = "control_points.vert.spv";
    inline constexpr auto kControlPointsFragmentShaderFile = "control_points.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainTessControlShaderKey = "tescMain";
    inline constexpr auto kMainTessEvaluationShaderKey = "teseMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";

    // Scene Objects
    inline constexpr auto kRootObject = "root";

    // Models
    inline constexpr auto kSuzanneModelPath = "Models/Suzanne.glb";
    inline constexpr auto kLowDetailSuzanneModelName = "lowDetailSuzanne";
    inline constexpr auto kMediumDetailSuzanneModelName = "mediumDetailSuzanne";
    inline constexpr auto kHighDetailSuzanneModelName = "highDetailSuzanne";

    // Object groups
    inline constexpr auto kLowDetail = "lowDetailGroup";
    inline constexpr auto kMediumDetail = "mediumDetailGroup";
    inline constexpr auto kHighDetail = "highDetailGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraZoomSpeed = "AppSettings.CameraZoomSpeed";
    constexpr auto PolygonMode = "AppSettings.PolygonMode";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::tessellation_shaders::curved_pn_triangles_tessellation
