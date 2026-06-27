/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    27.06.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_shadows::shadow_resource_management::layered_shadow_maps
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
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";
    inline constexpr auto kShadowMapsImage = "shadowMapsImage";
    inline constexpr auto kShadowMapsImageView = "shadowMapsImageView";
    inline constexpr auto kShadowMapLayerImageView = "shadowMapLayerImageView";

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

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kObject = "object";
    inline constexpr auto kFloorObject = "floor";
    inline constexpr auto kSpotLightObject1 = "spotLight1";
    inline constexpr auto kSpotLightObject2 = "spotLight2";
    inline constexpr auto kSpotLightObject3 = "spotLight3";

    // Object Groups
    inline constexpr auto kLightGroup = "lights";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::real_time_shadows::shadow_resource_management::layered_shadow_maps
