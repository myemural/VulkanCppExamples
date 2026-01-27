/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    28.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::pipelines_and_passes::multiple_render_passes
{
namespace constants
{
    // Shaders
    inline constexpr auto kBackgroundVertexShaderFile = "passthrough_position_uv.vert.spv";
    inline constexpr auto kBackgroundFragmentShaderFile = "texture_sampler_shift_with_time.frag.spv";
    inline constexpr auto kCubeVertexShaderFile = "position_with_mvp.vert.spv";
    inline constexpr auto kCubeFragmentShaderFile = "texture_sampler.frag.spv";
    inline constexpr auto kBackgroundVertexShaderKey = "vertBackground";
    inline constexpr auto kBackgroundFragmentShaderKey = "fragBackground";
    inline constexpr auto kCubeVertexShaderKey = "vertCube";
    inline constexpr auto kCubeFragmentShaderKey = "fragCube";

    // Buffers
    inline constexpr auto kCubeVertexBuffer = "cubeVertexBuffer";
    inline constexpr auto kCubeIndexBuffer = "cubeIndexBuffer";
    inline constexpr auto kPlaneVertexBuffer = "planeVertexBuffer";
    inline constexpr auto kPlaneIndexBuffer = "planeIndexBuffer";
    inline constexpr auto kTimeSpeedUniformBuffer = "timeSpeedUniformBuffer";

    // Images and Image Views
    inline constexpr auto kCrateImage = "crateImage";
    inline constexpr auto kCrateImageView = "crateImageView";
    inline constexpr auto kCloudImage = "cloudImage";
    inline constexpr auto kCloudImageView = "cloudImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kBackgroundDescSetLayout = "backgroundDescSetLayout";
    inline constexpr auto kCubeDescSetLayout = "cubeDescSetLayout";

    // Textures
    inline constexpr auto kCrateTexturePath = "Textures/crate1_diffuse.png";
    inline constexpr auto kCloudTexturePath = "Textures/cloud.png";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto CloudSpeed = "AppSettings.CloudSpeed";
} // namespace AppSettings
} // namespace examples::fundamentals::pipelines_and_passes::multiple_render_passes
