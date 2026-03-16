/**
 * @file    MemoryUtils.h
 * @brief   This file contains memory related utility functions.
 * @author  Mustafa Yemural (myemural)
 * @date    07.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

namespace common::utility
{

template<typename T>
void AppendBytes(std::vector<std::uint8_t>& buffer, const T& value)
{
    const auto* ptr = reinterpret_cast<const std::uint8_t*>(&value);
    buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
}

} // namespace common::utility