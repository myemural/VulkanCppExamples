/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    19.04.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_shadows::shadow_map_splitting_warping::cascaded_shadow_maps
{
namespace constants
{
    // Shaders
    inline constexpr auto kSceneVertexShaderFile = "scene.vert.spv";
    inline constexpr auto kSceneFragmentShaderFile = "scene.frag.spv";
    inline constexpr auto kShadowMapVertexShaderFile = "shadow_map_depth.vert.spv";
    inline constexpr auto kSceneVertexShaderKey = "vertScene";
    inline constexpr auto kSceneFragmentShaderKey = "fragScene";
    inline constexpr auto kShadowMapVertexShaderKey = "vertShadowMap";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kShadowMapImage = "shadowMapImage";
    inline constexpr auto kShadowMapImageView = "shadowMapImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kShadowSampler = "shadowSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kShadowMapDescSet = "shadowMapDescSet";
    inline constexpr auto kShadowMapDescSetLayout = "shadowMapDescSetLayout";

    // Textures
    inline constexpr auto kWoodFloorTexturePath = "Textures/Wood_Floor_015_basecolor.png";
    inline constexpr auto kWoodFloorTexture = "woodFloorTexture";
    inline constexpr auto kWoodFloorNormalTexturePath = "Textures/Wood_Floor_015_normal.png";
    inline constexpr auto kWoodFloorNormalTexture = "woodFloorNormalTexture";
    inline constexpr auto kTerracottaTexturePath = "Textures/Terracotta_Floor_Tiles_003_basecolor.png";
    inline constexpr auto kTerracottaTexture = "terracottaTexture";
    inline constexpr auto kTerracottaNormalTexturePath = "Textures/Terracotta_Floor_Tiles_003_normal.png";
    inline constexpr auto kTerracottaNormalTexture = "terracottaNormalTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kCubeObject1 = "cube1";
    inline constexpr auto kCubeObject2 = "cube2";
    inline constexpr auto kSphereObject = "sphere";
    inline constexpr auto kConeObject = "cone";
    inline constexpr auto kFloorObject = "floor";
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
} // namespace examples::real_time_shadows::shadow_map_splitting_warping::cascaded_shadow_maps
