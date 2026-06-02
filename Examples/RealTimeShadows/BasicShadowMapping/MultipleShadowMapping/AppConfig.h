/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    04.04.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_shadows::basic_shadow_mapping::multiple_shadow_mapping
{
namespace constants
{
    // Shaders
    inline constexpr auto kSceneVertexShaderFile = "scene.vert.spv";
    inline constexpr auto kSceneFragmentShaderFile = "scene.frag.spv";
    inline constexpr auto kLightObjectsFragmentShaderFile = "hardcoded_color_for_lights.frag.spv";
    inline constexpr auto kShadowMapVertexShaderFile = "shadow_map_depth.vert.spv";
    inline constexpr auto kShadowMapFragmentShaderFile = "shadow_map_depth.frag.spv";
    inline constexpr auto kSceneVertexShaderKey = "vertScene";
    inline constexpr auto kSceneFragmentShaderKey = "fragScene";
    inline constexpr auto kLightObjectsFragmentShaderKey = "fragLight";
    inline constexpr auto kShadowMapVertexShaderKey = "vertShadowMap";
    inline constexpr auto kShadowMapFragmentShaderKey = "fragShadowMap";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kPointLightShadowCubemapImage = "pointLightShadowCubemapImage";
    inline constexpr auto kPointLightShadowCubemapImageView = "pointLightShadowCubemapImageView";
    inline constexpr auto kPointLightShadowCubemapImageViewRight = "pointLightShadowCubemapImageViewRight";
    inline constexpr auto kPointLightShadowCubemapImageViewLeft = "pointLightShadowCubemapImageViewLeft";
    inline constexpr auto kPointLightShadowCubemapImageViewTop = "pointLightShadowCubemapImageViewTop";
    inline constexpr auto kPointLightShadowCubemapImageViewBottom = "pointLightShadowCubemapImageViewBottom";
    inline constexpr auto kPointLightShadowCubemapImageViewBack = "pointLightShadowCubemapImageViewBack";
    inline constexpr auto kPointLightShadowCubemapImageViewFront = "pointLightShadowCubemapImageViewFront";
    inline constexpr auto kDirLightShadowMapImage = "dirLightShadowMapImage";
    inline constexpr auto kDirLightShadowMapImageView = "dirLightShadowMapImageView";
    inline constexpr auto kSpotLightShadowMapImage = "spotLightShadowMapImage";
    inline constexpr auto kSpotLightShadowMapImageView = "spotLightShadowMapImageView";
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
    inline constexpr auto kObject = "object";
    inline constexpr auto kPointLightObject = "pointLight";
    inline constexpr auto kSpotLightObject = "spotLight";

    // Object Groups
    inline constexpr auto kLightGroup = "lights";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::real_time_shadows::basic_shadow_mapping::multiple_shadow_mapping
