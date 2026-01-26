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

namespace examples::fundamentals::descriptor_sets::basic_push_constants
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "passthrough_position.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "pushconst_color.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";

    // Buffers
    inline constexpr auto kMainVertexBuffer = "mainVertexBuffer";
    inline constexpr auto kMainIndexBuffer = "mainIndexBuffer";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
} // namespace AppSettings
} // namespace examples::fundamentals::descriptor_sets::basic_push_constants
