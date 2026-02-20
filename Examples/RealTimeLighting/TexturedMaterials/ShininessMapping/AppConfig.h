/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    24.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::textured_materials::shininess_mapping
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "blinn_phong_point.vert.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "blinn_phong_point.frag.spv";
    inline constexpr auto kLightObjectsFragmentShaderFile = "hardcoded_color_for_lights.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kSceneObjectsFragmentShaderKey = "fragScene";
    inline constexpr auto kLightObjectsFragmentShaderKey = "fragLight";

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
    inline constexpr auto kRockTexturePath = "Textures/Rock_Ore_001_COLOR.jpg";
    inline constexpr auto kRockTexture = "rockTexture";
    inline constexpr auto kRockSpecTexturePath = "Textures/Rock_Ore_001_SPEC.jpg";
    inline constexpr auto kRockSpecTexture = "rockSpecTexture";
    inline constexpr auto kRockRoughTexturePath = "Textures/Rock_Ore_001_ROUGH.jpg";
    inline constexpr auto kRockRoughTexture = "rockRoughTexture";

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
    constexpr auto ConstantFactor = "AppSettings.ConstantFactor";
    constexpr auto LinearFactor = "AppSettings.LinearFactor";
    constexpr auto QuadraticFactor = "AppSettings.QuadraticFactor";
} // namespace AppSettings
} // namespace examples::real_time_lighting::textured_materials::shininess_mapping
