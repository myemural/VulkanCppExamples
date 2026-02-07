/**
 * @file    AssetHandle.h
 * @brief   Contains asset handling related core types.
 * @author  Mustafa Yemural (myemural)
 * @date    07.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <cstdint>

namespace common::asset_manager
{

using AssetID = std::uint32_t;

template<typename AssetType>
struct AssetHandle
{
    AssetID id = 0;

    [[nodiscard]] bool IsValid() const { return id != 0; }
};

} // namespace common::asset_manager
