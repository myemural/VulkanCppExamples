/**
 * @file    TextureAsset.h
 * @brief   Contains texture asset data classes.
 * @author  Mustafa Yemural (myemural)
 * @date    07.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <vector>

#include "CoreDefines.h"

namespace common::asset_manager
{

struct COMMON_API TextureAsset
{
    std::vector<unsigned char> data;
    std::uint32_t width = UINT32_MAX;
    std::uint32_t height = UINT32_MAX;
    std::uint32_t channels = UINT32_MAX;
};

struct COMMON_API TextureAssetHDR
{
    std::vector<float> data;
    std::uint32_t width = UINT32_MAX;
    std::uint32_t height = UINT32_MAX;
    std::uint32_t channels = UINT32_MAX;
};

} // namespace common::asset_manager
