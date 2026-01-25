/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    14.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::images_and_samplers::using_multiple_textures
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "position_with_offset.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "multiple_texture_sampler.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";

    // Buffers
    inline constexpr auto kMainVertexBuffer = "mainVertexBuffer";
    inline constexpr auto kMainIndexBuffer = "mainIndexBuffer";
    inline constexpr auto kBricksStagingBuffer = "bricksStagingBuffer";
    inline constexpr auto kWallStagingBuffer = "wallStagingBuffer";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";

    // Textures
    inline constexpr auto kBricksTexturePath = "Textures/bricks.jpg";
    inline constexpr auto kWallTexturePath = "Textures/wall.jpg";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
} // namespace AppSettings
} // namespace examples::fundamentals::images_and_samplers::using_multiple_textures
