/**
 * @file    TextureLoader.h
 * @brief   Contains texture asset loader class implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    07.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <filesystem>

#include "AssetLoader.h"
#include "CoreDefines.h"
#include "TextureAsset.h"

namespace common::asset_manager
{

class COMMON_API TextureLoader : public AssetLoader<TextureAsset>
{
public:
    /**
     * @param basePath Base path of the textures.
     */
    explicit TextureLoader(std::filesystem::path basePath);

    /**
     * @brief Loads texture from file and returns it as an asset.
     * @param relativePath Relative path of the texture file.
     * @return Texture asset.
     */
    std::unique_ptr<TextureAsset> Load(const std::string& relativePath) override;

private:
    std::filesystem::path basePath_;
};

class COMMON_API TextureLoaderHDR : public AssetLoader<TextureAssetHDR>
{
public:
    /**
     * @param basePath Base path of the HDR textures.
     */
    explicit TextureLoaderHDR(std::filesystem::path basePath);

    /**
     * @brief Loads HDR texture from file and returns it as an asset.
     * @param relativePath Relative path of the HDR texture file.
     * @return HDR texture asset.
     */
    std::unique_ptr<TextureAssetHDR> Load(const std::string& relativePath) override;

private:
    std::filesystem::path basePath_;
};

} // namespace common::asset_manager
