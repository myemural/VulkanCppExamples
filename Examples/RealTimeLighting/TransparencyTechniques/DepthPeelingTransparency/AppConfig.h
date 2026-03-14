/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    14.03.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::transparency_techniques::depth_peeling_transparency
{
namespace constants
{
    // Shaders
    inline constexpr auto kSceneVertexShaderFile = "scene.vert.spv";
    inline constexpr auto kOpaqueFragmentShaderFile = "opaque_pass.frag.spv";
    inline constexpr auto kDepthPeelingFragmentShaderFile = "depth_peeling_pass.frag.spv";
    inline constexpr auto kCompositionVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kCompositionFragmentShaderFile = "composition_pass.frag.spv";
    inline constexpr auto kSceneVertexShaderKey = "vertScene";
    inline constexpr auto kOpaqueFragmentShaderKey = "fragOpaque";
    inline constexpr auto kDepthPeelingFragmentShaderKey = "fragDepthPeeling";
    inline constexpr auto kCompositionVertexShaderKey = "vertComposition";
    inline constexpr auto kCompositionFragmentShaderKey = "fragComposition";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kDepthOpaqueImage = "depthOpaqueImage";
    inline constexpr auto kDepthOpaqueImageView = "depthOpaqueImageView";
    inline constexpr auto kDepthPeelImage = "depthPeelImage";
    inline constexpr auto kDepthPeelImageView = "depthPeelImageView";
    inline constexpr auto kDepthPeelColorImage = "depthPeelColorImage";
    inline constexpr auto kDepthPeelColorImageView = "depthPeelColorImageView";
    inline constexpr auto kColorPeelImage = "colorPeelImage";
    inline constexpr auto kColorPeelImageView = "colorPeelImageView";
    inline constexpr auto kOpaqueColorImage = "opaqueColorImage";
    inline constexpr auto kOpaqueColorImageView = "opaqueColorImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kPeelingDescSet = "peelingDescSet";
    inline constexpr auto kPeelingDescSetLayout = "peelingDescSetLayout";
    inline constexpr auto kCompositionDescSet = "compositionDescSet";
    inline constexpr auto kCompositionDescSetLayout = "compositionDescSetLayout";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kPlaneObject = "plane";
    inline constexpr auto kCubeObject = "cube";

    // Scene Object Tags
    inline constexpr auto kTransparentObjectGroup = "transparentObjectGroup";
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
    constexpr auto PeelLayerCount = "AppSettings.PeelLayerCount";
} // namespace AppSettings
} // namespace examples::real_time_lighting::transparency_techniques::depth_peeling_transparency
