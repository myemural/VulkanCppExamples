/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    11.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "AppCommonConfig.h"

namespace examples::fundamentals::basics::clear_screen_with_color
{
namespace AppConstants
{
    constexpr auto MaxFramesInFlight = "AppConstants.MaxFramesInFlight";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
} // namespace AppSettings
} // namespace examples::fundamentals::basics::clear_screen_with_color
