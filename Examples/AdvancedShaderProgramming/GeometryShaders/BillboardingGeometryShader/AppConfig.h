/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    16.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::geometry_shaders::billboarding_geometry_shader
{
namespace constants
{
    // Shaders
    inline constexpr auto kSceneVertexShaderFile = "scene.vert.spv";
    inline constexpr auto kSceneFragmentShaderFile = "scene.frag.spv";
    inline constexpr auto kBillboardingVertexShaderFile = "billboarding.vert.spv";
    inline constexpr auto kBillboardingGeometryShaderFile = "billboarding.geom.spv";
    inline constexpr auto kBillboardingFragmentShaderFile = "billboarding.frag.spv";
    inline constexpr auto kSceneVertexShaderKey = "vertScene";
    inline constexpr auto kSceneFragmentShaderKey = "fragScene";
    inline constexpr auto kBillboardingVertexShaderKey = "vertBillboard";
    inline constexpr auto kBillboardingGeometryShaderKey = "geomBillboard";
    inline constexpr auto kBillboardingFragmentShaderKey = "fragBillboard";

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
    inline constexpr auto kGroundTexturePath = "Textures/Ground_Dirt_007_basecolor.jpg";
    inline constexpr auto kGroundTexture = "groundTexture";
    inline constexpr auto kGroundNormalTexturePath = "Textures/Ground_Dirt_007_normal.jpg";
    inline constexpr auto kGroundNormalTexture = "groundNormalTexture";
    inline constexpr auto kTreeTexturePath = "Textures/horse-chestnut-tree_0.png";
    inline constexpr auto kTreeTexture = "treeTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kGroundPlane = "groundPlane";
    inline constexpr auto kBillboardPoint = "billboardPoint";

    // Object groups
    inline constexpr auto kBillboardGroup = "billboardGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::geometry_shaders::billboarding_geometry_shader
