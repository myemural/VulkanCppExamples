/**
 * @file    ModelLoader.h
 * @brief   Contains 3D model asset loader class implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    30.03.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <filesystem>

#include "AssetLoader.h"
#include "CoreDefines.h"
#include "ModelAsset.h"

namespace common::asset_manager
{

class COMMON_API ModelLoader : public AssetLoader<GltfModelAsset>
{
public:
    /**
     * @param basePath Base path of the models.
     */
    explicit ModelLoader(std::filesystem::path basePath);

    /**
     * @brief Loads glTF model from file and returns it as an asset.
     * @param relativePath Relative path of the model file.
     * @return glTF model asset.
     */
    std::unique_ptr<GltfModelAsset> Load(const std::string& relativePath) override;

private:
    std::filesystem::path basePath_;
    tinygltf::TinyGLTF gltfLoader_;
};

} // namespace common::asset_manager
