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
namespace AppConstants
{
    // Shaders
    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto MainFragmentShaderFile = "AppConstants.MainFragmentShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto MainFragmentShaderKey = "AppConstants.MainFragmentShaderKey";

    // Resources
    constexpr auto MainVertexBuffer = "AppConstants.MainVertexBuffer";
    constexpr auto MainIndexBuffer = "AppConstants.MainIndexBuffer";
    constexpr auto TopLeftUB = "AppConstants.TopLeftUB";
    constexpr auto TopRightUB = "AppConstants.TopRightUB";
    constexpr auto BottomLeftUB = "AppConstants.BottomLeftUB";
    constexpr auto BottomRightUB = "AppConstants.BottomRightUB";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
} // namespace AppSettings
} // namespace examples::fundamentals::descriptor_sets::array_of_ub
