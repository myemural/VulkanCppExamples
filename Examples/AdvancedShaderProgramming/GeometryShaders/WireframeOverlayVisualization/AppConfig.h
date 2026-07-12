/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    12.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::geometry_shaders::wireframe_overlay_visualization
{
namespace constants
{
    // Shaders
    inline constexpr auto kSceneVertexShaderFile = "scene.vert.spv";
    inline constexpr auto kSceneFragmentShaderFile = "scene.frag.spv";
    inline constexpr auto kWireframeVisualVertexShaderFile = "wireframe_visualization.vert.spv";
    inline constexpr auto kWireframeVisualGeometryShaderFile = "wireframe_visualization.geom.spv";
    inline constexpr auto kWireframeVisualFragmentShaderFile = "hardcoded_color.frag.spv";
    inline constexpr auto kSceneVertexShaderKey = "vertScene";
    inline constexpr auto kSceneFragmentShaderKey = "fragScene";
    inline constexpr auto kWireframeVisualVertexShaderKey = "vertWireframeVisual";
    inline constexpr auto kWireframeVisualGeometryShaderKey = "geomWireframeVisual";
    inline constexpr auto kWireframeVisualFragmentShaderKey = "fragWireframeVisual";

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
    inline constexpr auto kWireframeVisualDescSet = "wireframeVisualDescSet";
    inline constexpr auto kWireframeVisualDescSetLayout = "wireframeVisualDescSetLayout";

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
    constexpr auto WireframeLineWidth = "AppSettings.WireframeLineWidth";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::geometry_shaders::wireframe_overlay_visualization
