/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ModelLoader.h"

#include <iostream>
#include <filesystem>

#include <stb_image.h>

#include "TextureHandler.h"

namespace common::utility
{

std::shared_ptr<GltfModelHandler> ModelLoader::LoadBinaryGltfFromFile(const std::string& filePath)
{
    tinygltf::Model gltfModel;
    std::string error;
    std::string warning;

    if (filePath.find_last_of("glb") == std::string::npos) {
        std::cerr << "File does not a glb file!" << std::endl;
        return nullptr;
    }

    if (!gltfLoader_.LoadBinaryFromFile(&gltfModel, &error, &warning, filePath)) {
        std::cerr << "GLTF file could not be loaded: " << error << std::endl;
        return nullptr;
    }

    if (!warning.empty()) {
        std::cout << "GLTF load warning: " << warning << std::endl;
    }

    auto gltfModelHandler = std::make_shared<GltfModelHandler>();
    gltfModelHandler->Name = GenerateModelName(filePath);

    if (!ProcessTextures(gltfModelHandler, gltfModel)) {
        std::cerr << "GLTF processing textures error!" << std::endl;
        return nullptr;
    }

    if (!ProcessMaterials(gltfModelHandler, gltfModel)) {
        std::cerr << "GLTF processing materials error!" << std::endl;
        return nullptr;
    }

    if (!ProcessMeshes(gltfModelHandler, gltfModel)) {
        std::cerr << "GLTF processing meshes error!" << std::endl;
        return nullptr;
    }

    return gltfModelHandler;
}

std::shared_ptr<GltfModelHandler> ModelLoader::LoadAsciiGltfFromFile(const std::string& filePath)
{
    tinygltf::Model gltfModel;
    std::string error;
    std::string warning;

    if (!gltfLoader_.LoadASCIIFromFile(&gltfModel, &error, &warning, filePath)) {
        std::cerr << "GLTF file could not be loaded: " << error << std::endl;
        return nullptr;
    }

    if (!warning.empty()) {
        std::cout << "GLTF load warning: " << warning << std::endl;
    }

    auto gltfModelHandler = std::make_shared<GltfModelHandler>();
    gltfModelHandler->Name = GenerateModelName(filePath);

    if (!ProcessTextures(gltfModelHandler, gltfModel)) {
        std::cerr << "GLTF processing textures error!" << std::endl;
        return nullptr;
    }

    if (!ProcessMaterials(gltfModelHandler, gltfModel)) {
        std::cerr << "GLTF processing materials error!" << std::endl;
        return nullptr;
    }

    if (!ProcessMeshes(gltfModelHandler, gltfModel)) {
        std::cerr << "GLTF processing meshes error!" << std::endl;
        return nullptr;
    }

    return gltfModelHandler;
}

std::string ModelLoader::GenerateModelName(const std::string& filePath)
{
    std::filesystem::path path = filePath;
    std::string fileName = path.stem().string();
    return fileName;
}

bool ModelLoader::ProcessTextures(const std::shared_ptr<GltfModelHandler>& handler, const tinygltf::Model& gltfModel)
{
    for (const auto& tex: gltfModel.textures) {
        if (const int texIndex = tex.source; texIndex >= 0 && texIndex < gltfModel.images.size()) {
            TextureHandler textureHandler;
            if (const auto& image = gltfModel.images[texIndex]; image.uri.empty()) {
                textureHandler.Width = image.width;
                textureHandler.Height = image.height;
                textureHandler.Channels = image.component;
                textureHandler.Data = image.image;
            } else {
                /// TODO: uri existance will be handled later
                std::cerr << "GLTF image uri handling is not supported for now!" << std::endl;
                return false;
            }

            handler->Textures.push_back(textureHandler);
        } else {
            std::cerr << "GLTF texture index is wrong!" << std::endl;
            return false;
        }
    }

    return true;
}

bool ModelLoader::ProcessMaterials(const std::shared_ptr<GltfModelHandler>& handler, const tinygltf::Model& gltfModel)
{
    static int i = 0;
    for (const auto& mat: gltfModel.materials) {
        GltfMaterial material;
        std::string matName = mat.name.empty() ? "mat" + std::to_string(i++) : mat.name;
        material.Name = handler->Name + "_" + matName;
        if (mat.values.contains("baseColorTexture")) {
            material.PbrMetallicRoughness.BaseColorTextureIndex = mat.values.at("baseColorTexture").TextureIndex();
        }

        handler->Materials.push_back(material);
    }

    return true;
}

bool ModelLoader::ProcessMeshes(const std::shared_ptr<GltfModelHandler>& handler, const tinygltf::Model& gltfModel)
{
    static int meshCount = 0;
    for (const auto& mesh: gltfModel.meshes) {
        for (const auto& primitive: mesh.primitives) {
            GltfMesh gltfMesh;
            std::string meshName = mesh.name.empty() ? "mesh" + std::to_string(meshCount++) : mesh.name;
            gltfMesh.Name = handler->Name + "_" + meshName;

            // Vertex Positions
            std::vector<float> posData;
            if (primitive.attributes.contains("POSITION")) {
                const auto& posAccessor = gltfModel.accessors[primitive.attributes.at("POSITION")];
                const auto& posBufferView = gltfModel.bufferViews[posAccessor.bufferView];
                const auto& posBuffer = gltfModel.buffers[posBufferView.buffer];

                size_t start = posBufferView.byteOffset + posAccessor.byteOffset;
                size_t length = posAccessor.count * tinygltf::GetNumComponentsInType(posAccessor.type);

                posData.resize(length);
                std::memcpy(posData.data(), &posBuffer.data[start], length * sizeof(float));
            } else {
                std::cerr << "GLTF primitive should contain POSITION attribute!" << std::endl;
                return false;
            }

            // Vertex Normals
            std::vector<float> normData;
            if (primitive.attributes.contains("NORMAL")) {
                const auto& normAccessor = gltfModel.accessors[primitive.attributes.at("NORMAL")];
                const auto& normBufferView = gltfModel.bufferViews[normAccessor.bufferView];
                const auto& normBuffer = gltfModel.buffers[normBufferView.buffer];

                size_t start = normBufferView.byteOffset + normAccessor.byteOffset;
                size_t length = normAccessor.count * tinygltf::GetNumComponentsInType(normAccessor.type);

                normData.resize(length);
                std::memcpy(normData.data(), &normBuffer.data[start], length * sizeof(float));
            }

            // Vertex TexCoords (only TEXCOORD_0 for now)
            std::vector<float> texData;
            if (primitive.attributes.contains("TEXCOORD_0")) {
                const auto& texAccessor = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
                const auto& texBufferView = gltfModel.bufferViews[texAccessor.bufferView];
                const auto& texBuffer = gltfModel.buffers[texBufferView.buffer];

                size_t start = texBufferView.byteOffset + texAccessor.byteOffset;
                size_t length = texAccessor.count * tinygltf::GetNumComponentsInType(texAccessor.type);

                texData.resize(length);
                std::memcpy(texData.data(), &texBuffer.data[start], length * sizeof(float));
            }

            // Process vertices
            const auto& posAccessor = gltfModel.accessors[primitive.attributes.at("POSITION")];
            for (size_t i = 0; i < posAccessor.count; ++i) {
                GltfPrimitiveAttrib attribute;
                attribute.Position = glm::vec3(posData[i * 3 + 0], posData[i * 3 + 1], posData[i * 3 + 2]);
                if (!normData.empty()) {
                    attribute.Normal = glm::vec3(normData[i * 3 + 0], normData[i * 3 + 1], normData[i * 3 + 2]);
                }
                if (!texData.empty()) {
                    attribute.TexCoords.emplace_back(glm::vec2(texData[i * 2 + 0], texData[i * 2 + 1]));
                }
                gltfMesh.Vertices.push_back(attribute);
            }

            // Indices
            const auto& idxAccessor = gltfModel.accessors[primitive.indices];
            const auto& idxBufferView = gltfModel.bufferViews[idxAccessor.bufferView];
            const auto& idxBuffer = gltfModel.buffers[idxBufferView.buffer];

            if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                size_t start = idxBufferView.byteOffset + idxAccessor.byteOffset;
                size_t length = idxAccessor.count * sizeof(uint16_t);

                gltfMesh.Indices.resize(length);
                std::memcpy(gltfMesh.Indices.data(), &idxBuffer.data[start], length);
            } else {
                std::cerr << "GLTF unsupported index type!" << std::endl;
                return false;
            }

            gltfMesh.MaterialIndex = primitive.material;
            handler->Meshes.push_back(gltfMesh);
        }
    }

    return true;
}

} // namespace common::utility
