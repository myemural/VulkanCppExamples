/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    29.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::pipelines_and_passes::offscreen_rendering
{
namespace constants
{
    // Shaders
    inline constexpr auto kQuadVertexShaderFile = "passthrough_position_uv.vert.spv";
    inline constexpr auto kSceneVertexShaderFile = "position_with_mvp.vert.spv";
    inline constexpr auto kQuadFragmentShaderFile = "texture_sampler_with_border.frag.spv";
    inline constexpr auto kSceneFragmentShaderFile = "texture_sampler.frag.spv";
    inline constexpr auto kQuadVertexShaderKey = "vertQuad";
    inline constexpr auto kSceneVertexShaderKey = "vertScene";
    inline constexpr auto kQuadFragmentShaderKey = "fragQuad";
    inline constexpr auto kSceneFragmentShaderKey = "fragScene";

    // Buffers
    inline constexpr auto kCubeVertexBuffer = "cubeVertexBuffer";
    inline constexpr auto kCubeIndexBuffer = "cubeIndexBuffer";
    inline constexpr auto kPlaneVertexBuffer = "planeVertexBuffer";
    inline constexpr auto kPlaneIndexBuffer = "planeIndexBuffer";

    // Images and Image Views
    inline constexpr auto kCrateImage = "crateImage";
    inline constexpr auto kCrateImageView = "crateImageView";
    inline constexpr auto kOffscreenImage = "offscreenImage";
    inline constexpr auto kOffscreenImageView = "offscreenImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kSceneDescSetLayout = "sceneDescSetLayout";
    inline constexpr auto kQuadDescSetLayout = "quadDescSetLayout";

    // Textures
    inline constexpr auto kCrateTexturePath = "Textures/crate1_diffuse.png";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::fundamentals::pipelines_and_passes::offscreen_rendering
