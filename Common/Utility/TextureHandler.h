/**
 * @file    TextureHandler.h
 * @brief   This file contains texture handler related types.
 * @author  Mustafa Yemural (myemural)
 * @date    12.10.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <cstdint>
#include <vector>

#include "CoreDefines.h"

namespace common::utility
{

enum class TextureChannelFormat
{
    RGB,
    RGBA
};

struct COMMON_API TextureHandler
{
    std::vector<unsigned char> Data;
    std::uint32_t Width = UINT32_MAX;
    std::uint32_t Height = UINT32_MAX;
    std::uint32_t Channels = UINT32_MAX;
    TextureChannelFormat Format = TextureChannelFormat::RGBA;
};

} // namespace common::utility
