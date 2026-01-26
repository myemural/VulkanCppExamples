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

namespace examples::fundamentals::descriptor_sets::changing_color_with_ub
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "passthrough_position.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "ubo_color.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";

    // Buffers
    inline constexpr auto kMainVertexBuffer = "mainVertexBuffer";
    inline constexpr auto kMainUniformBuffer = "mainUniformBuffer";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto TriangleColor = "AppSettings.TriangleColor";
} // namespace AppSettings
} // namespace examples::fundamentals::descriptor_sets::changing_color_with_ub
