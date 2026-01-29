/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    2.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::multisampling::explicit_resolving
{
namespace constants
{
    // Shaders
    inline constexpr auto kQuadVertexShaderFile = "passthrough_position_uv.vert.spv";
    inline constexpr auto kSceneVertexShaderFile = "position_with_mvp.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "texture_sampler.frag.spv";
    inline constexpr auto kQuadVertexShaderKey = "vertQuad";
    inline constexpr auto kSceneVertexShaderKey = "vertScene";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";

    // Buffers
    inline constexpr auto kSphereVertexBuffer = "sphereVertexBuffer";
    inline constexpr auto kSphereIndexBuffer = "sphereIndexBuffer";
    inline constexpr auto kPlaneVertexBuffer = "planeVertexBuffer";
    inline constexpr auto kPlaneIndexBuffer = "planeIndexBuffer";

    // Images and Image Views
    inline constexpr auto kMarbleImage = "marbleImage";
    inline constexpr auto kMarbleImageView = "marbleImageView";
    inline constexpr auto kMultisampledImage = "multisampledImage";
    inline constexpr auto kMultisampledImageView = "multisampledImageView";
    inline constexpr auto kResolvedImage = "resolvedImage";
    inline constexpr auto kResolvedImageView = "resolvedImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kSceneDescSetLayout = "sceneDescSetLayout";
    inline constexpr auto kQuadDescSetLayout = "quadDescSetLayout";

    // Textures
    inline constexpr auto kMarbleTexturePath = "Textures/marble.jpg";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::fundamentals::multisampling::explicit_resolving
