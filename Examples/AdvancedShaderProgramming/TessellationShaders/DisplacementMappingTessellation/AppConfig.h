/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    21.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::tessellation_shaders::basic_displacement_tessellation
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "blinn_phong_directional.vert.spv";
    inline constexpr auto kMainTessControlShaderFile = "displacement_tessellation.tesc.spv";
    inline constexpr auto kMainTessEvaluationShaderFile = "displacement_tessellation.tese.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "blinn_phong_directional.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainTessControlShaderKey = "tescMain";
    inline constexpr auto kMainTessEvaluationShaderKey = "teseMain";
    inline constexpr auto kSceneObjectsFragmentShaderKey = "fragScene";

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
    inline constexpr auto kWallStoneTexturePath = "Textures/Wall_Stone_025_basecolor.png";
    inline constexpr auto kWallStoneTexture = "wallStoneTexture";
    inline constexpr auto kWallStoneNormalTexturePath = "Textures/Wall_Stone_025_normal.png";
    inline constexpr auto kWallStoneNormalTexture = "wallStoneNormalTexture";
    inline constexpr auto kWallStoneHeightTexturePath = "Textures/Wall_Stone_025_height.png";
    inline constexpr auto kWallStoneHeightTexture = "wallStoneHeightTexture";

    inline constexpr auto kMetalPatternTexturePath = "Textures/Metal_Pattern_012_basecolor.png";
    inline constexpr auto kMetalPatternTexture = "metalPatternTexture";
    inline constexpr auto kMetalPatternNormalTexturePath = "Textures/Metal_Pattern_012_normal.png";
    inline constexpr auto kMetalPatternNormalTexture = "metalPatternNormalTexture";
    inline constexpr auto kMetalPatternHeightTexturePath = "Textures/Metal_Pattern_012_height.png";
    inline constexpr auto kMetalPatternHeightTexture = "metalPatternHeightTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kSphereObject1 = "sphere1";
    inline constexpr auto kSphereObject2 = "sphere2";
    inline constexpr auto kSphereObject3 = "sphere3";
    inline constexpr auto kSphereObject4 = "sphere4";
    inline constexpr auto kPlane = "plane";

    // Object Groups
    inline constexpr auto kSphericalObject = "sphericalObjectGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto PolygonMode = "AppSettings.PolygonMode";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::tessellation_shaders::basic_displacement_tessellation
