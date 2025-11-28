/**
 * @file    ScopedTimer.h
 * @brief   Scoped timer and some profiling utility implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    28.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <chrono>

#include "CoreDefines.h"

namespace common::utility
{

class COMMON_API ScopedTimer
{
public:
    using clock = std::chrono::steady_clock;

    explicit ScopedTimer(const std::string& name, long long printIntervalMs = 0);

    ~ScopedTimer();

private:
    std::string name_;
    long long printIntervalMs_;
    clock::time_point startTime_;
};

} // namespace common::utility

#define PROFILE_SCOPE(name) ScopedTimer _scoped_timer_##__LINE__(name)
#define PROFILE_SCOPE_EVERY(name, ms) ScopedTimer _scoped_timer_##__LINE__(name, ms)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)
#define PROFILE_FUNCTION_EVERY(ms) PROFILE_SCOPE_EVERY(__FUNCTION__, ms)
