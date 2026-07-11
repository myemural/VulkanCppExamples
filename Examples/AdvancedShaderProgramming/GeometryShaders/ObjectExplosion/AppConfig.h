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

namespace examples::advanced_shader_programming::geometry_shaders::object_explosion
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "specular_blinn_phong.vert.spv";
    inline constexpr auto kMainGeometryShaderFile = "object_explosion.geom.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "specular_blinn_phong.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainGeometryShaderKey = "geomMain";
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

    // Scene Objects
    inline constexpr auto kRootObject = "root";

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
} // namespace examples::advanced_shader_programming::geometry_shaders::object_explosion
