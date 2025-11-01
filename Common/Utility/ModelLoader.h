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

#include "CoreDefines.h"
#include "GlfwModelHandler.h"

namespace common::utility
{

class COMMON_API ModelLoader
{
public:
    /**
     * @param basePath Base path of the model.
     */
    explicit ModelLoader(std::string basePath);

    ~ModelLoader() = default;

    /**
     * @brief Reads glTF binary model from file and returns the data with tha handler.
     * @param filePath File path of the binary glTF model.
     * @return Returns glTF model handler.
     */
    [[nodiscard]] std::shared_ptr<GltfModelHandler> LoadBinaryGltfFromFile(const std::string& filePath);

    /**
     * @brief Reads glTF ASCII model from file and returns the data with tha handler.
     * @param filePath File path of the ASCII glTF model.
     * @return Returns glTF model handler.
     */
    [[nodiscard]] std::shared_ptr<GltfModelHandler> LoadAsciiGltfFromFile(const std::string& filePath);

private:
    [[nodiscard]] std::shared_ptr<GltfModelHandler> ProcessGltfModel(const std::string& filePath) const;

    [[nodiscard]] bool ProcessTextures(const std::shared_ptr<GltfModelHandler>& handler,
                                       const std::string& parentPath = "") const;

    [[nodiscard]] bool ProcessMaterials(const std::shared_ptr<GltfModelHandler>& handler) const;

    [[nodiscard]] bool ProcessMeshes(const std::shared_ptr<GltfModelHandler>& handler) const;

    [[nodiscard]] bool ProcessNodes(const std::shared_ptr<GltfModelHandler>& handler) const;

    [[nodiscard]] bool ProcessCameras(const std::shared_ptr<GltfModelHandler>& handler) const;

    std::string basePath_;
    tinygltf::TinyGLTF gltfLoader_;
    tinygltf::Model gltfModel_;
};

} // namespace common::utility
