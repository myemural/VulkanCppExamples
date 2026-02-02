/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    22.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::textured_materials::emissive_materials
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
    inline constexpr auto kCeilingTexturePath = "Textures/Ceiling_Gypsum_001_Base_Color.jpg";
    inline constexpr auto kCeilingTexture = "ceilingTexture";
    inline constexpr auto kCeilingEmissiveTexturePath = "Textures/Ceiling_Gypsum_001_emissive.jpg";
    inline constexpr auto kCeilingEmissiveTexture = "ceilingEmissiveTexture";

    // Materials
    inline constexpr auto kDefaultMaterial = "defaultMaterial";

    // Scene Objects
    inline constexpr auto kCameraObject = "camera";
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
    constexpr auto ConstantFactor = "AppSettings.ConstantFactor";
    constexpr auto LinearFactor = "AppSettings.LinearFactor";
    constexpr auto QuadraticFactor = "AppSettings.QuadraticFactor";
} // namespace AppSettings
} // namespace examples::real_time_lighting::textured_materials::emissive_materials
