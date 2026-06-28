/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    28.06.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_shadows::shadow_resource_management::anisotropic_shadow_maps
{
namespace constants
{
    // Shaders
    inline constexpr auto kSceneVertexShaderFile = "scene.vert.spv";
    inline constexpr auto kSceneFragmentShaderFile = "scene.frag.spv";
    inline constexpr auto kShadowMapVertexShaderFile = "shadow_map_depth.vert.spv";
    inline constexpr auto kShadowMapFragmentShaderFile = "shadow_map_depth.frag.spv";
    inline constexpr auto kSceneVertexShaderKey = "vertScene";
    inline constexpr auto kSceneFragmentShaderKey = "fragScene";
    inline constexpr auto kShadowMapVertexShaderKey = "vertShadowMap";
    inline constexpr auto kShadowMapFragmentShaderKey = "fragShadowMap";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kShadowMapImage = "shadowMapImage";
    inline constexpr auto kShadowMapImageViewMip0 = "shadowMapImageViewMip0";
    inline constexpr auto kShadowMapImageView = "shadowMapImageView";
    inline constexpr auto kShadowMapDepthImage = "shadowMapDepthImage";
    inline constexpr auto kShadowMapDepthImageView = "shadowMapDepthImageView";
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

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kCubeObject = "cube";
    inline constexpr auto kSphereObject = "sphere";
    inline constexpr auto kCylinderObject = "cylinder";
    inline constexpr auto kFloorObject = "floor";

    // Models
    inline constexpr auto kAntiqueCameraModelPath = "Models/AntiqueCamera.glb";
    inline constexpr auto kAntiqueCameraModelName = "AntiqueCamera";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::real_time_shadows::shadow_resource_management::anisotropic_shadow_maps
