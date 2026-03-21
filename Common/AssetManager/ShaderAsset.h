/**
 * @file    ShaderAsset.h
 * @brief   Contains shader asset data classes.
 * @author  Mustafa Yemural (myemural)
 * @date    07.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <cstdint>
#include <vector>

#include "CoreDefines.h"

namespace common::asset_manager
{

struct COMMON_API ShaderAsset
{
    std::vector<std::uint32_t> data;
};

} // namespace common::asset_manager
