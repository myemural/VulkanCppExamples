/**
 * @file    ModelLoader.h
 * @brief   This file contains utility class for loading model data from file.
 * @author  Mustafa Yemural (myemural)
 * @date    6.10.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <memory>
#include <string>

#include "tiny_gltf.h"

#include "GlfwModelHandler.h"

namespace common::utility
{

class ModelLoader
{
public:
    [[nodiscard]] std::shared_ptr<GltfModelHandler> LoadBinaryGltfFromFile(const std::string& filePath);

    [[nodiscard]] std::shared_ptr<GltfModelHandler> LoadAsciiGltfFromFile(const std::string& filePath);

private:
    std::string GenerateModelName(const std::string& filePath);

    bool ProcessTextures(const std::shared_ptr<GltfModelHandler>& handler, const tinygltf::Model& gltfModel);

    bool ProcessMaterials(const std::shared_ptr<GltfModelHandler>& handler, const tinygltf::Model& gltfModel);

    bool ProcessMeshes(const std::shared_ptr<GltfModelHandler>& handler, const tinygltf::Model& gltfModel);

    bool ProcessNodes(const std::shared_ptr<GltfModelHandler>& handler, const tinygltf::Model& gltfModel);

    tinygltf::TinyGLTF gltfLoader_;
};

} // namespace common::utility
