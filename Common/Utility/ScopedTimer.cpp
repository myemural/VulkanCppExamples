/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ScopedTimer.h"

#include <iostream>

namespace common::utility
{

ScopedTimer::ScopedTimer(const std::string& name, const long long printIntervalMs)
    : name_(name), printIntervalMs_(printIntervalMs), startTime_(clock::now())
{
}

ScopedTimer::~ScopedTimer()
{
    using namespace std::chrono;

    const auto endTime = clock::now();
    const long long us = duration_cast<microseconds>(endTime - startTime_).count();

    if (printIntervalMs_ > 0) {
        static auto lastPrint = clock::now();

        const auto now = endTime;

        if (const auto elapsedMs = duration_cast<milliseconds>(now - lastPrint).count(); elapsedMs < printIntervalMs_) {
            return;
        }

        lastPrint = now;
    }

    std::cout << name_ << ": " << us << " microseconds" << std::endl;
}

} // namespace common::utility
