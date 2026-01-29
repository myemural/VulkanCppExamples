/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    9.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::compute_shaders::checkerboard_generator
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "position_with_mvp.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "texture_sampler.frag.spv";
    inline constexpr auto kCheckerboardComputeShaderFile = "checkerboard_generator.comp.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";
    inline constexpr auto kCheckerboardComputeShaderKey = "compCheckerboard";

    // Buffers
    inline constexpr auto kCubeVertexBuffer = "cubeVertexBuffer";
    inline constexpr auto kCubeIndexBuffer = "cubeIndexBuffer";
    inline constexpr auto kSphereVertexBuffer = "sphereVertexBuffer";
    inline constexpr auto kSphereIndexBuffer = "sphereIndexBuffer";

    // Images and Image Views
    inline constexpr auto kCheckerboardStorageImage = "checkerboardStorageImage";
    inline constexpr auto kCheckerboardStorageImageView = "checkerboardStorageImageView";
    inline constexpr auto kCrateImage = "crateImage";
    inline constexpr auto kCrateImageView = "crateImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kCubeDescSet = "cubeDescSet";
    inline constexpr auto kSphereDescSet = "sphereDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kComputeDescSetLayout = "computeDescSetLayout";

    // Textures
    inline constexpr auto kCrateTexturePath = "Textures/crate1_diffuse.png";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto CheckerboardTextureSize = "AppSettings.CheckerboardTextureSize";
    constexpr auto TileSize = "AppSettings.TileSize";
} // namespace AppSettings
} // namespace examples::fundamentals::compute_shaders::checkerboard_generator
