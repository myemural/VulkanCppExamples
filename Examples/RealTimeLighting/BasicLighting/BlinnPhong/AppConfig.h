/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    14.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::basic_lighting::blinn_phong
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "specular_blinn_phong.vert.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "specular_blinn_phong.frag.spv";
    inline constexpr auto kLightObjectsFragmentShaderFile = "hardcoded_color_for_lights.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kSceneObjectsFragmentShaderKey = "fragScene";
    inline constexpr auto kLightObjectsFragmentShaderKey = "fragLight";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";

    // Materials
    inline constexpr auto kDefaultMaterial = "defaultMaterial";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kCubeObject = "cube";
    inline constexpr auto kSphereObject = "sphere";
    inline constexpr auto kConeObject = "cone";
    inline constexpr auto kCylinderObject = "cylinder";
    inline constexpr auto kPlaneObject = "plane";
    inline constexpr auto kLightObject = "light";

    // Object Groups
    inline constexpr auto kLightGroup = "lights";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto LightColor = "AppSettings.LightColor";
    constexpr auto AmbientStrength = "AppSettings.AmbientStrength";
    constexpr auto SpecularStrength = "AppSettings.SpecularStrength";
    constexpr auto Shininess = "AppSettings.Shininess";
} // namespace AppSettings
} // namespace examples::real_time_lighting::basic_lighting::blinn_phong
