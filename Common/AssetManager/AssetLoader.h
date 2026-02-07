/**
 * @file    AssetLoader.h
 * @brief   Contains base classes and interfaces which related to asset loading mechanism.
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

namespace common::asset_manager
{

class IAssetLoader
{
public:
    virtual ~IAssetLoader() = default;
};

template<typename T>
class AssetLoader : public IAssetLoader
{
public:
    virtual std::unique_ptr<T> Load(const std::string& relativePath) = 0;
};

} // namespace common::asset_manager
