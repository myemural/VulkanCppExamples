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
std::vector<std::uint8_t> StructToByteVector(const T* data)
{
    static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable!");

    std::vector<std::uint8_t> bytes(sizeof(T));
    std::memcpy(bytes.data(), data, sizeof(T));
    return bytes;
}

template<typename T>
void AppendBytes(std::vector<std::uint8_t>& buffer, const T& value)
{
    const auto* ptr = reinterpret_cast<const std::uint8_t*>(&value);
    buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
}

constexpr std::uint32_t Align16(const std::uint32_t size) { return (size + 15u) & ~15u; }

} // namespace common::utility