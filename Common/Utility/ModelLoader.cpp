/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ModelLoader.h"

#include <filesystem>
#include <iostream>

#include <utility>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "TextureLoader.h"

namespace common::utility
{
namespace
{
    std::string GenerateModelName(const std::string& filePath)
    {
        const std::filesystem::path path = filePath;
        std::string fileName = path.stem().string();
        return fileName;
    }

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

    void ComputeWorldTransform(std::vector<GltfNode>& nodes,
                               const std::uint32_t nodeIndex,
                               const glm::mat4& parentWorldMatrix)
    {
        auto& node = nodes[nodeIndex];
        node.worldTransform = parentWorldMatrix * node.localTransform;

        for (const auto childIndex: node.childIndices) {
            ComputeWorldTransform(nodes, childIndex, node.worldTransform);
        }
    }
} // namespace

ModelLoader::ModelLoader(std::string basePath) : basePath_{std::move(basePath)} {}

std::shared_ptr<GltfModelHandler> ModelLoader::LoadBinaryGltfFromFile(const std::string& filePath)
{
    std::string error;
    std::string warning;

    if (filePath.find_last_of("glb") == std::string::npos) {
        std::cerr << "File does not a glb file!" << std::endl;
        return nullptr;
    }

    if (!gltfLoader_.LoadBinaryFromFile(&gltfModel_, &error, &warning, basePath_ + filePath)) {
        std::cerr << "GLTF file could not be loaded: " << error << std::endl;
        return nullptr;
    }

    if (!error.empty()) {
        std::cout << "GLTF load error: " << error << std::endl;
    }

    if (!warning.empty()) {
        std::cout << "GLTF load warning: " << warning << std::endl;
    }

    return ProcessGltfModel(filePath);
}

std::shared_ptr<GltfModelHandler> ModelLoader::LoadAsciiGltfFromFile(const std::string& filePath)
{
    std::string error;
    std::string warning;

    if (!gltfLoader_.LoadASCIIFromFile(&gltfModel_, &error, &warning, basePath_ + filePath)) {
        std::cerr << "GLTF file could not be loaded: " << error << std::endl;
        return nullptr;
    }

    if (!error.empty()) {
        std::cout << "GLTF load error: " << error << std::endl;
    }

    if (!warning.empty()) {
        std::cout << "GLTF load warning: " << warning << std::endl;
    }

    return ProcessGltfModel(filePath);
}

std::shared_ptr<GltfModelHandler> ModelLoader::ProcessGltfModel(const std::string& filePath) const
{
    auto gltfModelHandler = std::make_shared<GltfModelHandler>();
    gltfModelHandler->name = GenerateModelName(filePath);

    const std::string parentPath = (std::filesystem::path{filePath}.parent_path() / "").string();
    if (!ProcessTextures(gltfModelHandler, parentPath)) {
        std::cerr << "GLTF processing textures error!" << std::endl;
        return nullptr;
    }

    if (!ProcessMaterials(gltfModelHandler)) {
        std::cerr << "GLTF processing materials error!" << std::endl;
        return nullptr;
    }

    if (!ProcessMeshes(gltfModelHandler)) {
        std::cerr << "GLTF processing meshes error!" << std::endl;
        return nullptr;
    }

    if (!ProcessNodes(gltfModelHandler)) {
        std::cerr << "GLTF processing nodes error!" << std::endl;
        return nullptr;
    }

    if (!ProcessCameras(gltfModelHandler)) {
        std::cerr << "GLTF processing cameras error!" << std::endl;
        return nullptr;
    }

    return gltfModelHandler;
}

bool ModelLoader::ProcessTextures(const std::shared_ptr<GltfModelHandler>& handler, const std::string& parentPath) const
{
    for (const auto& tex: gltfModel_.textures) {
        if (const int texIndex = tex.source; texIndex >= 0 && texIndex < gltfModel_.images.size()) {
            asset_manager::TextureAsset textureAsset;
            if (const auto& image = gltfModel_.images[texIndex]; image.uri.empty()) {
                textureAsset.width = image.width;
                textureAsset.height = image.height;
                textureAsset.channels = image.component;
                textureAsset.data = image.image;
            } else {
                asset_manager::TextureLoader textureLoader{""};
                textureAsset = *textureLoader.Load(parentPath + image.uri);
            }

            handler->textures.push_back(textureAsset);
        } else {
            std::cerr << "GLTF texture index is wrong!" << std::endl;
            return false;
        }
    }

    return true;
}

bool ModelLoader::ProcessMaterials(const std::shared_ptr<GltfModelHandler>& handler) const
{
    static int i = 0;
    for (const auto& mat: gltfModel_.materials) {
        GltfMaterial material;
        std::string matName = mat.name.empty() ? "mat" + std::to_string(i++) : mat.name;
        material.name = handler->name + "_" + matName;

        // pbrMetallicRoughness
        const auto& baseColor = mat.pbrMetallicRoughness.baseColorFactor;
        material.pbrMetallicRoughness.baseColor = {static_cast<float>(baseColor[0]), static_cast<float>(baseColor[1]),
                                                   static_cast<float>(baseColor[2]), static_cast<float>(baseColor[3])};
        material.pbrMetallicRoughness.baseColorTextureIndex = mat.pbrMetallicRoughness.baseColorTexture.index;
        material.pbrMetallicRoughness.metallicFactor = static_cast<float>(mat.pbrMetallicRoughness.metallicFactor);
        material.pbrMetallicRoughness.roughnessFactor = static_cast<float>(mat.pbrMetallicRoughness.roughnessFactor);
        material.pbrMetallicRoughness.metallicRoughnessTextureIndex =
                mat.pbrMetallicRoughness.metallicRoughnessTexture.index;

        // normalTexture
        material.normalTextureInfo.index = mat.normalTexture.index;
        material.normalTextureInfo.scale = static_cast<float>(mat.normalTexture.scale);

        // occlusionTexture
        material.occlusionTextureInfo.index = mat.occlusionTexture.index;
        material.occlusionTextureInfo.strength = static_cast<float>(mat.occlusionTexture.strength);

        // emissive
        material.emissionTextureIndex = mat.emissiveTexture.index;
        const auto& emissiveFactor = mat.emissiveFactor;
        material.emissiveFactor = {static_cast<float>(emissiveFactor[0]), static_cast<float>(emissiveFactor[1]),
                                   static_cast<float>(emissiveFactor[2])};

        handler->materials.push_back(material);
    }

    return true;
}

bool ModelLoader::ProcessMeshes(const std::shared_ptr<GltfModelHandler>& handler) const
{
    static int meshCount = 0;
    for (const auto& mesh: gltfModel_.meshes) {
        for (const auto& primitive: mesh.primitives) {
            GltfMesh gltfMesh;
            std::string meshName = mesh.name.empty() ? "mesh" + std::to_string(meshCount++) : mesh.name;
            gltfMesh.name = handler->name + "_" + meshName;

            // Vertex Positions
            std::vector<glm::vec3> posData;
            if (primitive.attributes.contains("POSITION")) {
                const auto& posAccessor = gltfModel_.accessors[primitive.attributes.at("POSITION")];
                const auto& posBufferView = gltfModel_.bufferViews[posAccessor.bufferView];
                const auto& posBuffer = gltfModel_.buffers[posBufferView.buffer];

                size_t start = posBufferView.byteOffset + posAccessor.byteOffset;
                size_t length = posAccessor.count;

                posData.resize(length);
                std::memcpy(posData.data(), &posBuffer.data[start], length * sizeof(glm::vec3));
            } else {
                std::cerr << "GLTF primitive should contain POSITION attribute!" << std::endl;
                return false;
            }

            // Vertex Normals
            std::vector<glm::vec3> normData;
            if (primitive.attributes.contains("NORMAL")) {
                const auto& normAccessor = gltfModel_.accessors[primitive.attributes.at("NORMAL")];
                const auto& normBufferView = gltfModel_.bufferViews[normAccessor.bufferView];
                const auto& normBuffer = gltfModel_.buffers[normBufferView.buffer];

                size_t start = normBufferView.byteOffset + normAccessor.byteOffset;
                size_t length = normAccessor.count;

                normData.resize(length);
                std::memcpy(normData.data(), &normBuffer.data[start], length * sizeof(glm::vec3));
            }

            // Vertex Normals
            std::vector<glm::vec4> tangentData;
            if (primitive.attributes.contains("TANGENT")) {
                const auto& tangentAccessor = gltfModel_.accessors[primitive.attributes.at("TANGENT")];
                const auto& tangentBufferView = gltfModel_.bufferViews[tangentAccessor.bufferView];
                const auto& tangentBuffer = gltfModel_.buffers[tangentBufferView.buffer];

                size_t start = tangentBufferView.byteOffset + tangentAccessor.byteOffset;
                size_t length = tangentAccessor.count;

                tangentData.resize(length);
                std::memcpy(tangentData.data(), &tangentBuffer.data[start], length * sizeof(glm::vec4));
            }

            // Vertex TexCoords (only TEXCOORD_0 for now)
            std::vector<glm::vec2> texData;
            if (primitive.attributes.contains("TEXCOORD_0")) {
                const auto& texAccessor = gltfModel_.accessors[primitive.attributes.at("TEXCOORD_0")];
                const auto& texBufferView = gltfModel_.bufferViews[texAccessor.bufferView];
                const auto& texBuffer = gltfModel_.buffers[texBufferView.buffer];

                size_t start = texBufferView.byteOffset + texAccessor.byteOffset;
                size_t length = texAccessor.count;

                texData.resize(length);
                std::memcpy(texData.data(), &texBuffer.data[start], length * sizeof(glm::vec2));
            }

            // Process vertices
            GltfPrimitiveAttrib attribute;
            attribute.vertexCount = posData.size();
            attribute.positions = std::move(posData);
            attribute.normals = std::move(normData);
            attribute.tangents = std::move(tangentData);
            attribute.texCoords0 = std::move(texData);
            gltfMesh.attributes = std::move(attribute);

            // Indices
            const auto& idxAccessor = gltfModel_.accessors[primitive.indices];
            const auto& idxBufferView = gltfModel_.bufferViews[idxAccessor.bufferView];
            const auto& idxBuffer = gltfModel_.buffers[idxBufferView.buffer];

            if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                size_t start = idxBufferView.byteOffset + idxAccessor.byteOffset;
                size_t length = idxAccessor.count * sizeof(std::uint16_t);

                gltfMesh.indices.resize(length);
                std::memcpy(gltfMesh.indices.data(), &idxBuffer.data[start], length);
            } else {
                std::cerr << "GLTF unsupported index type!" << std::endl;
                return false;
            }

            gltfMesh.materialIndex = primitive.material;
            handler->meshes.push_back(gltfMesh);
        }
    }

    return true;
}

bool ModelLoader::ProcessNodes(const std::shared_ptr<GltfModelHandler>& handler) const
{
    std::vector<GltfNode> gltfNodes{gltfModel_.nodes.size()};

    // Set mesh index, child indices, camera index and local transform
    for (size_t i = 0; i < gltfModel_.nodes.size(); ++i) {
        if (gltfModel_.nodes[i].mesh != -1) {
            gltfNodes[i].meshIndex = gltfModel_.nodes[i].mesh;
        }
        for (const auto child: gltfModel_.nodes[i].children) {
            gltfNodes[i].childIndices.emplace_back(static_cast<std::uint32_t>(child));
        }
        gltfNodes[i].cameraIndex = gltfModel_.nodes[i].camera != -1 ? gltfModel_.nodes[i].camera : UINT32_MAX;
        gltfNodes[i].localTransform = GetLocalTransform(gltfModel_.nodes[i]);
    }

    // Set parents
    for (size_t i = 0; i < gltfModel_.nodes.size(); ++i) {
        for (const auto childIndex: gltfModel_.nodes[i].children) {
            gltfNodes[childIndex].parentIndex = static_cast<std::uint32_t>(i);
        }
    }

    // Calculate world transforms
    handler->currentSceneIndex = gltfModel_.defaultScene > -1 ? gltfModel_.defaultScene : 0;
    for (const auto& scene = gltfModel_.scenes[handler->currentSceneIndex]; const auto rootNode: scene.nodes) {
        ComputeWorldTransform(gltfNodes, static_cast<std::uint32_t>(rootNode), glm::mat4(1.0f));
    }

    handler->nodes = gltfNodes;

    return true;
}

bool ModelLoader::ProcessCameras(const std::shared_ptr<GltfModelHandler>& handler) const
{
    for (const auto& camera: gltfModel_.cameras) {
        GltfCamera gltfCamera;
        gltfCamera.name = camera.name;
        gltfCamera.type = camera.type == "perspective" ? GltfCameraType::PERSPECTIVE : GltfCameraType::ORTHOGRAPHIC;

        if (gltfCamera.type == GltfCameraType::PERSPECTIVE) {
            gltfCamera.perspectiveFeatures.aspectRatio = static_cast<float>(camera.perspective.aspectRatio);
            gltfCamera.perspectiveFeatures.fov = static_cast<float>(glm::degrees(camera.perspective.yfov));
            gltfCamera.perspectiveFeatures.near = static_cast<float>(camera.perspective.znear);
            gltfCamera.perspectiveFeatures.far = static_cast<float>(camera.perspective.zfar);
        } else {
            gltfCamera.orthographicFeatures.aspectRatio =
                    static_cast<float>(camera.orthographic.xmag / camera.orthographic.ymag);
            gltfCamera.orthographicFeatures.size =
                    static_cast<float>(std::max(camera.orthographic.xmag, camera.orthographic.ymag));
            gltfCamera.orthographicFeatures.near = static_cast<float>(camera.orthographic.znear);
            gltfCamera.orthographicFeatures.far = static_cast<float>(camera.orthographic.zfar);
        }

        handler->cameras.push_back(gltfCamera);
    }

    return true;
}

} // namespace common::utility
