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
    explicit TextureLoader(std::filesystem::path basePath);

    std::unique_ptr<TextureAsset> Load(const std::string& relativePath) override;

private:
    std::filesystem::path basePath_;
};

} // namespace common::asset_manager
