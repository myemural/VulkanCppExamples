/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    27.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::pipelines_and_passes::basic_stencil_testing
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "position_with_mvp.vert.spv";
    inline constexpr auto kObjectFragmentShaderFile = "texture_sampler.frag.spv";
    inline constexpr auto kOutlineFragmentShaderFile = "hardcoded_color.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kObjectFragmentShaderKey = "fragObject";
    inline constexpr auto kOutlineFragmentShaderKey = "fragOutline";

    // Buffers
    inline constexpr auto kMainVertexBuffer = "mainVertexBuffer";
    inline constexpr auto kMainIndexBuffer = "mainIndexBuffer";

    // Images and Image Views
    inline constexpr auto kCrateImage = "crateImage";
    inline constexpr auto kCrateImageView = "crateImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";

    // Textures
    inline constexpr auto kCrateTexturePath = "Textures/crate1_diffuse.png";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto OutlineWidth = "AppSettings.OutlineWidth";
} // namespace AppSettings
} // namespace examples::fundamentals::pipelines_and_passes::basic_stencil_testing
