/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "MathUtils.h"

namespace common::utility
{

std::uint32_t CeilDiv(const std::uint32_t value, const std::uint32_t divisor)
{
    return (value + (divisor - 1)) / divisor;
}

} // namespace common::utility
