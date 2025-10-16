/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ModelLoader.h"

#include <filesystem>
#include <iostream>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "TextureHandler.h"
#include "TextureLoader.h"

namespace common::utility
{
namespace
{
    glm::mat4 GetLocalTransform(const tinygltf::Node& node)
    {
        glm::mat4 mat(1.0f);

        if (!node.matrix.empty()) {
            for (int i = 0; i < 16; ++i) {
                mat[i / 4][i % 4] = static_cast<float>(node.matrix[i]);
            }
        } else {
            glm::vec3 translation(0.0f);
            glm::vec3 scale(1.0f);
            glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);

            if (node.translation.size() == 3) {
                translation = glm::make_vec3(node.translation.data());
            }
            if (node.scale.size() == 3) {
                scale = glm::make_vec3(node.scale.data());
            }
            if (node.rotation.size() == 4) {
                rotation = glm::make_quat(node.rotation.data());
            }

            mat = glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation) *
                  glm::scale(glm::mat4(1.0f), scale);
        }

        return mat;
    }

    void ComputeWorldTransform(std::vector<GltfNode>& nodes, std::uint32_t nodeIndex, const glm::mat4& parentWorld)
    {
        auto& node = nodes[nodeIndex];
        node.WorldTransform = parentWorld * node.LocalTransform;

        for (const auto childIndex: node.ChildIndices) {
            ComputeWorldTransform(nodes, childIndex, node.WorldTransform);
        }
    }
} // namespace

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

    if (!ProcessNodes(gltfModelHandler, gltfModel)) {
        std::cerr << "GLTF processing nodes error!" << std::endl;
        return nullptr;
    }

    if (!ProcessCameras(gltfModelHandler, gltfModel)) {
        std::cerr << "GLTF processing cameras error!" << std::endl;
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

    const std::string parentPath = (std::filesystem::path{filePath}.parent_path() / "").string();
    if (!ProcessTextures(gltfModelHandler, gltfModel, parentPath)) {
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

    if (!ProcessNodes(gltfModelHandler, gltfModel)) {
        std::cerr << "GLTF processing nodes error!" << std::endl;
        return nullptr;
    }

    if (!ProcessCameras(gltfModelHandler, gltfModel)) {
        std::cerr << "GLTF processing cameras error!" << std::endl;
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

bool ModelLoader::ProcessTextures(const std::shared_ptr<GltfModelHandler>& handler,
                                  const tinygltf::Model& gltfModel,
                                  const std::string& parentPath)
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
                TextureLoader textureLoader{""};
                textureHandler = textureLoader.Load(parentPath + image.uri);
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

bool ModelLoader::ProcessNodes(const std::shared_ptr<GltfModelHandler>& handler, const tinygltf::Model& gltfModel)
{
    std::vector<GltfNode> gltfNodes{gltfModel.nodes.size()};

    // Set mesh index, child indices, camera index and local transform
    for (size_t i = 0; i < gltfModel.nodes.size(); ++i) {
        if (gltfModel.nodes[i].mesh != -1) {
            gltfNodes[i].MeshIndex = gltfModel.nodes[i].mesh;
        }
        for (const auto child: gltfModel.nodes[i].children) {
            gltfNodes[i].ChildIndices.emplace_back(static_cast<uint32_t>(child));
        }
        gltfNodes[i].CameraIndex = gltfModel.nodes[i].camera != -1 ? gltfModel.nodes[i].camera : UINT32_MAX;
        gltfNodes[i].LocalTransform = GetLocalTransform(gltfModel.nodes[i]);
    }

    // Set parents
    for (size_t i = 0; i < gltfModel.nodes.size(); ++i) {
        for (const auto childIndex: gltfModel.nodes[i].children) {
            gltfNodes[childIndex].ParentIndex = static_cast<std::uint32_t>(i);
        }
    }

    // Calculate world transforms
    handler->CurrentSceneIndex = gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0;
    const tinygltf::Scene& scene = gltfModel.scenes[handler->CurrentSceneIndex];
    for (const auto rootNode: scene.nodes) {
        ComputeWorldTransform(gltfNodes, static_cast<std::uint32_t>(rootNode), glm::mat4(1.0f));
    }

    handler->Nodes = gltfNodes;

    return true;
}

bool ModelLoader::ProcessCameras(const std::shared_ptr<GltfModelHandler>& handler, const tinygltf::Model& gltfModel)
{
    for (const auto& camera: gltfModel.cameras) {
        GltfCamera gltfCamera;
        gltfCamera.Name = camera.name;
        gltfCamera.Type = camera.type == "perspective" ? GltfCameraType::PERSPECTIVE : GltfCameraType::ORTHOGRAPHIC;

        if (gltfCamera.Type == GltfCameraType::PERSPECTIVE) {
            gltfCamera.PerspectiveFeatures.AspectRatio = static_cast<float>(camera.perspective.aspectRatio);
            gltfCamera.PerspectiveFeatures.Fov = static_cast<float>(glm::degrees(camera.perspective.yfov));
            gltfCamera.PerspectiveFeatures.Near = static_cast<float>(camera.perspective.znear);
            gltfCamera.PerspectiveFeatures.Far = static_cast<float>(camera.perspective.zfar);
        } else {
            gltfCamera.OrthographicFeatures.AspectRatio =
                    static_cast<float>(camera.orthographic.xmag / camera.orthographic.ymag);
            gltfCamera.OrthographicFeatures.Size =
                    static_cast<float>(std::max(camera.orthographic.xmag, camera.orthographic.ymag));
            gltfCamera.OrthographicFeatures.Near = static_cast<float>(camera.orthographic.znear);
            gltfCamera.OrthographicFeatures.Far = static_cast<float>(camera.orthographic.zfar);
        }

        handler->Cameras.push_back(gltfCamera);
    }

    return true;
}

} // namespace common::utility
