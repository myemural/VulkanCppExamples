/**
 * @file    MathUtils.h
 * @brief   This file contains math related utility functions.
 * @author  Mustafa Yemural (myemural)
 * @date    8.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <cstdint>

#include "CoreDefines.h"

namespace common::utility
{

/**
 * @brief Computes the ceiling division between two integral values.
 * @param value The value to be divided.
 * @param divisor The divisor.
 * @return Divides value by divisor. If value is not a multiple of divisor rounds the result up to the next integer
 * value.
 */
COMMON_API std::uint32_t CeilDiv(std::uint32_t value, std::uint32_t divisor);

} // namespace common::utility
