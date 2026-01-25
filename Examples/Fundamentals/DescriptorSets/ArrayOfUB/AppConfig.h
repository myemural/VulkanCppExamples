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

namespace examples::fundamentals::descriptor_sets::array_of_ub
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "position_with_indexed_ubo_model.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "hardcoded_color_from_index.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";

    // Buffers
    inline constexpr auto kMainVertexBuffer = "mainVertexBuffer";
    inline constexpr auto kMainIndexBuffer = "mainIndexBuffer";
    inline constexpr auto kTopLeftUB = "topLeftUB";
    inline constexpr auto kTopRightUB = "topRightUB";
    inline constexpr auto kBottomLeftUB = "bottomLeftUB";
    inline constexpr auto kBottomRightUB = "bottomRightUB";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
} // namespace AppSettings
} // namespace examples::fundamentals::descriptor_sets::array_of_ub
