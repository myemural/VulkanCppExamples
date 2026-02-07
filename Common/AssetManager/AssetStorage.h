/**
 * @file    AssetStorage.h
 * @brief   Contains classes and interfaces which related to asset cache management.
 * @author  Mustafa Yemural (myemural)
 * @date    07.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "AssetHandle.h"

namespace common::asset_manager
{

class IAssetStorage
{
public:
    virtual ~IAssetStorage() = default;
};

template<typename T>
class AssetStorage : public IAssetStorage
{
public:
    std::unordered_map<AssetID, std::unique_ptr<T>> assets;
    std::unordered_map<std::string, AssetID> pathToId;
};

} // namespace common::asset_manager
