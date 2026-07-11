/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    11.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::geometry_shaders::normal_vector_visualization
{
namespace constants
{
    // Shaders
    inline constexpr auto kSceneVertexShaderFile = "scene.vert.spv";
    inline constexpr auto kSceneFragmentShaderFile = "scene.frag.spv";
    inline constexpr auto kNormalVisualVertexShaderFile = "normal_visualization.vert.spv";
    inline constexpr auto kNormalVisualGeometryShaderFile = "normal_visualization.geom.spv";
    inline constexpr auto kNormalVisualFragmentShaderFile = "hardcoded_color.frag.spv";
    inline constexpr auto kSceneVertexShaderKey = "vertScene";
    inline constexpr auto kSceneFragmentShaderKey = "fragScene";
    inline constexpr auto kNormalVisualVertexShaderKey = "vertNormalVisual";
    inline constexpr auto kNormalVisualGeometryShaderKey = "geomNormalVisual";
    inline constexpr auto kNormalVisualFragmentShaderKey = "fragNormalVisual";

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
    inline constexpr auto kNormalVisualDescSet = "normalVisualDescSet";
    inline constexpr auto kNormalVisualDescSetLayout = "normalVisualDescSetLayout";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kCubeObject = "cube";
    inline constexpr auto kSphereObject = "sphere";
    inline constexpr auto kCylinderObject = "cylinder";
    inline constexpr auto kConeObject = "cone";
    inline constexpr auto kFloorObject = "floor";

    // Models
    inline constexpr auto kMilkTruckModelPath = "Models/CesiumMilkTruck.glb";
    inline constexpr auto kMilkTruckModelName = "MilkTruck";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::geometry_shaders::normal_vector_visualization
