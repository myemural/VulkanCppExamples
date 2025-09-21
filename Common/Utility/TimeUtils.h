/**
 * @file    TimeUtils.h
 * @brief   This file contains time related utility functions.
 * @author  Mustafa Yemural (myemural)
 * @date    21.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <chrono>

namespace common::utility
{
/**
 * @brief Returns current time in seconds.
 * @return Returns current time in seconds.
 */
inline double GetCurrentTime()
{
    using clock = std::chrono::steady_clock;
    return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}
} // namespace common::utility
