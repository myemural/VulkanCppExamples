/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    08.03.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::transparency_techniques::weighted_blended_transparency
{
namespace constants
{
    // Shaders
    inline constexpr auto kTransparencyVertexShaderFile = "scene_transparency.vert.spv";
    inline constexpr auto kTransparencyFragmentShaderFile = "scene_weighted_oit.frag.spv";
    inline constexpr auto kCompositionVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kCompositionFragmentShaderFile = "composition_pass.frag.spv";
    inline constexpr auto kTransparencyVertexShaderKey = "vertTransparency";
    inline constexpr auto kTransparencyFragmentShaderKey = "fragTransparency";
    inline constexpr auto kCompositionVertexShaderKey = "vertComposition";
    inline constexpr auto kCompositionFragmentShaderKey = "fragComposition";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kAccumulationImage = "accumulationImage";
    inline constexpr auto kAccumulationImageView = "accumulationImageView";
    inline constexpr auto kRevealageImage = "revealageImage";
    inline constexpr auto kRevealageImageView = "revealageImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kCompositionDescSet = "compositionDescSet";
    inline constexpr auto kCompositionDescSetLayout = "compositionDescSetLayout";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kPlaneObject = "plane";
    inline constexpr auto kCubeObject = "cube";
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
} // namespace examples::real_time_lighting::transparency_techniques::weighted_blended_transparency
