/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ModelLoader.h"

#include <iostream>
#include <utility>

namespace common::asset_manager
{

ModelLoader::ModelLoader(std::filesystem::path basePath) : basePath_(std::move(basePath)) {}

std::unique_ptr<GltfModelAsset> ModelLoader::Load(const std::string& relativePath)
{
    tinygltf::Model gltfModel;
    std::string error;
    std::string warning;

    if (relativePath.find(".gltf") != std::string::npos) {
        if (!gltfLoader_.LoadASCIIFromFile(&gltfModel, &error, &warning, (basePath_ / relativePath).string())) {
            throw std::runtime_error("ASCII GLTF file could not be loaded: " + relativePath);
        }
    } else if (relativePath.find(".glb") != std::string::npos) {
        if (!gltfLoader_.LoadBinaryFromFile(&gltfModel, &error, &warning, (basePath_ / relativePath).string())) {
            throw std::runtime_error("Binary GLTF file could not be loaded: " + relativePath);
        }
    } else {
        throw std::runtime_error("GLTF file could not be loaded because of wrong format: " + relativePath);
    }

    if (!error.empty()) {
        throw std::runtime_error("GLTF load error: " + error);
    }

    if (!warning.empty()) {
        std::cout << "GLTF load warning: " << warning << std::endl;
    }

    return std::make_unique<GltfModelAsset>(std::move(gltfModel));
}

} // namespace common::asset_manager
