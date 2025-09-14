/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    13.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "AppCommonConfig.h"

namespace examples::fundamentals::basics::drawing_with_different_topology
{
namespace AppConstants
{
    constexpr auto MaxFramesInFlight = "AppConstants.MaxFramesInFlight";
    constexpr auto BaseShaderType = "AppConstants.BaseShaderType";
    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto MainFragmentShaderFile = "AppConstants.MainFragmentShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto MainFragmentShaderKey = "AppConstants.MainFragmentShaderKey";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto PrimitiveRestartEnabled = "AppSettings.PrimitiveRestartEnabled";
    constexpr auto PolygonMode = "AppSettings.PolygonMode";
    constexpr auto LineWidth = "AppSettings.LineWidth";
} // namespace AppSettings
} // namespace examples::fundamentals::basics::drawing_with_different_topology
