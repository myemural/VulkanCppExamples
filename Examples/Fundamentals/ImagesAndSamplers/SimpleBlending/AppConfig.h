/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    14.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::images_and_samplers::simple_blending
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "position_with_offset.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "texture_sampler.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";

    // Buffers
    inline constexpr auto kMainVertexBuffer = "mainVertexBuffer";
    inline constexpr auto kMainIndexBuffer = "mainIndexBuffer";
    inline constexpr auto kImageStagingBuffer = "imageStagingBuffer";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";

    // Textures
    inline constexpr auto kLeafTexturePath = "Textures/leaf.png";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
} // namespace AppSettings
} // namespace examples::fundamentals::images_and_samplers::simple_blending
