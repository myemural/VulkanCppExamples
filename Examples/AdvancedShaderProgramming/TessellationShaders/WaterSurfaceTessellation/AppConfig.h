/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    27.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::tessellation_shaders::water_surface_tessellation
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "water_tessellation.vert.spv";
    inline constexpr auto kMainTessControlShaderFile = "water_tessellation.tesc.spv";
    inline constexpr auto kMainTessEvaluationShaderFile = "water_tessellation.tese.spv";
    inline constexpr auto kMainFragmentShaderFile = "water_tessellation.frag.spv";
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

    // Textures
    inline constexpr auto kWaterNormalTexturePath = "Textures/Water_002_NORM.jpg";
    inline constexpr auto kWaterNormalTexture = "waterNormalTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kWaterSurface = "waterSurface";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto PolygonMode = "AppSettings.PolygonMode";
    constexpr auto TessLevel = "AppSettings.TessLevel";
    constexpr auto DisplacementLevel = "AppSettings.DisplacementLevel";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::tessellation_shaders::water_surface_tessellation
