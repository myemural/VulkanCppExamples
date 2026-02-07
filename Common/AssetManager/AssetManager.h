/**
 * @file    AssetManager.h
 * @brief   Contains main asset manager class implementation.
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
#include <typeindex>
#include <unordered_map>

#include "AssetHandle.h"
#include "AssetLoader.h"
#include "AssetStorage.h"

namespace common::asset_manager
{

class AssetManager
{
public:
    AssetManager() = default;
    ~AssetManager() = default;

    template<typename T>
    void RegisterLoader(std::unique_ptr<AssetLoader<T>> loader)
    {
        const std::type_index type = typeid(T);

        loaders_[type] = std::move(loader);
        storages_[type] = std::make_unique<AssetStorage<T>>();
    }

    template<typename T>
    AssetHandle<T> Load(const std::string& logicalPath)
    {
        const std::type_index type = typeid(T);

        auto* storage = GetStorage<T>(type);
        auto* loader = GetLoader<T>(type);

        // Check cache
        auto it = storage->pathToId.find(logicalPath);
        if (it != storage->pathToId.end()) {
            return {it->second};
        }

        auto asset = loader->Load(logicalPath);
        if (!asset) {
            throw std::runtime_error("Asset loader error!");
        }

        AssetID id = nextId_++;
        storage->assets[id] = std::move(asset);
        storage->pathToId[logicalPath] = id;

        return {id};
    }

    template<typename T>
    const T& Get(const AssetHandle<T>& handle) const
    {
        const std::type_index type = typeid(T);

        auto* storage = GetStorage<T>(type);

        auto it = storage->assets.find(handle.id);
        if (it == storage->assets.end()) {
            throw std::runtime_error("Invalid asset!");
        }

        return *it->second;
    }

private:
    template<typename T>
    AssetStorage<T>* GetStorage(const std::type_index& type)
    {
        const auto it = storages_.find(type);
        if (it == storages_.end()) {
            throw std::runtime_error("No storage registered for asset type!");
        }

        return static_cast<AssetStorage<T>*>(it->second.get());
    }

    template<typename T>
    const AssetStorage<T>* GetStorage(const std::type_index& type) const
    {
        const auto it = storages_.find(type);
        if (it == storages_.end()) {
            throw std::runtime_error("No storage registered for asset type!");
        }

        return static_cast<const AssetStorage<T>*>(it->second.get());
    }

    template<typename T>
    AssetLoader<T>* GetLoader(const std::type_index& type)
    {
        const auto it = loaders_.find(type);
        if (it == loaders_.end()) {
            throw std::runtime_error("No loader registered for asset type!");
        }

        return static_cast<AssetLoader<T>*>(it->second.get());
    }

    AssetID nextId_ = 1;
    std::unordered_map<std::type_index, std::unique_ptr<IAssetLoader>> loaders_;
    std::unordered_map<std::type_index, std::unique_ptr<IAssetStorage>> storages_;
};

} // namespace common::asset_manager
