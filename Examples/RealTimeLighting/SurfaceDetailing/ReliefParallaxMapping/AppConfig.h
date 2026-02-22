/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    09.01.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::surface_detailing::relief_parallax_mapping
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "blinn_phong_directional.vert.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "blinn_phong_directional.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
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
    inline constexpr auto kPebblesTexturePath = "Textures/Pebbles_027_BaseColor.jpg";
    inline constexpr auto kPebblesTexture = "pebblesTexture";
    inline constexpr auto kPebblesNormalTexturePath = "Textures/Pebbles_027_Normal.jpg";
    inline constexpr auto kPebblesNormalTexture = "pebblesNormalTexture";
    inline constexpr auto kPebblesHeightTexturePath = "Textures/Pebbles_027_Height.png";
    inline constexpr auto kPebblesHeightTexture = "pebblesHeightTexture";

    // Materials
    inline constexpr auto kDefaultMaterial = "defaultMaterial";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kCubeObject = "cube";
    inline constexpr auto kPlaneObjectBottom = "planeBottom";
    inline constexpr auto kPlaneObjectBack = "planeBack";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto LightDirection = "AppSettings.LightDirection";
    constexpr auto LightColor = "AppSettings.LightColor";
    constexpr auto AmbientStrength = "AppSettings.AmbientStrength";
    constexpr auto SpecularStrength = "AppSettings.SpecularStrength";
    constexpr auto Shininess = "AppSettings.Shininess";
} // namespace AppSettings
} // namespace examples::real_time_lighting::surface_detailing::relief_parallax_mapping
