/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ModelAsset.h"

#include <stdexcept>

#include <meshoptimizer.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace common::asset_manager
{

std::vector<utility::VertexPos3Uv2> GltfModelAsset::GetVertices(const std::uint32_t meshIndex,
                                                                const std::uint32_t primitiveIndex) const
{
    const auto& primitive = gltfModel_.meshes[meshIndex].primitives[primitiveIndex];
    std::vector<utility::VertexPos3Uv2> result;

    // Vertex Positions
    std::vector<glm::vec3> posData;
    if (primitive.attributes.contains("POSITION")) {
        const auto& posAccessor = gltfModel_.accessors[primitive.attributes.at("POSITION")];
        const auto& posBufferView = gltfModel_.bufferViews[posAccessor.bufferView];
        const auto& posBuffer = gltfModel_.buffers[posBufferView.buffer];

        const size_t start = posBufferView.byteOffset + posAccessor.byteOffset;
        const size_t length = posAccessor.count;

        posData.resize(length);
        std::memcpy(posData.data(), &posBuffer.data[start], length * sizeof(glm::vec3));
    } else {
        throw std::runtime_error("GLTF primitive does not contain POSITION attribute!");
    }

    // Vertex TexCoords
    std::vector<glm::vec2> texData;
    if (primitive.attributes.contains("TEXCOORD_0")) {
        const auto& texAccessor = gltfModel_.accessors[primitive.attributes.at("TEXCOORD_0")];
        const auto& texBufferView = gltfModel_.bufferViews[texAccessor.bufferView];
        const auto& texBuffer = gltfModel_.buffers[texBufferView.buffer];

        const size_t start = texBufferView.byteOffset + texAccessor.byteOffset;
        const size_t length = texAccessor.count;

        texData.resize(length);
        std::memcpy(texData.data(), &texBuffer.data[start], length * sizeof(glm::vec2));
    }

    for (auto i = 0U; i < posData.size(); ++i) {
        utility::VertexPos3Uv2 current{};
        current.Position.data = posData[i];
        current.Uv.data.x = texData[i].x;
        current.Uv.data.y = texData[i].y;
        result.emplace_back(current);
    }

    return result;
}

std::vector<std::uint16_t> GltfModelAsset::GetIndices(const std::uint32_t meshIndex,
                                                      const std::uint32_t primitiveIndex) const
{
    const auto& primitive = gltfModel_.meshes[meshIndex].primitives[primitiveIndex];
    std::vector<std::uint16_t> result;

    // Indices
    const auto& idxAccessor = gltfModel_.accessors[primitive.indices];
    const auto& idxBufferView = gltfModel_.bufferViews[idxAccessor.bufferView];
    const auto& idxBuffer = gltfModel_.buffers[idxBufferView.buffer];

    if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        const size_t start = idxBufferView.byteOffset + idxAccessor.byteOffset;
        const size_t length = idxAccessor.count * sizeof(std::uint16_t);

        result.resize(idxAccessor.count);
        std::memcpy(result.data(), &idxBuffer.data[start], length);
    } else {
        throw std::runtime_error("GLTF unsupported index type!");
    }

    return result;
}

std::uint32_t GltfModelAsset::GetVertexBufferSize(const std::uint32_t meshIndex,
                                                  const std::uint32_t primitiveIndex) const
{
    const auto& primitive = gltfModel_.meshes[meshIndex].primitives[primitiveIndex];
    const auto& posAccessor = gltfModel_.accessors[primitive.attributes.at("POSITION")];
    return posAccessor.count * sizeof(utility::VertexPos3Uv2);
}

std::uint32_t GltfModelAsset::GetIndexBufferSize(const std::uint32_t meshIndex,
                                                 const std::uint32_t primitiveIndex) const
{
    const auto& primitive = gltfModel_.meshes[meshIndex].primitives[primitiveIndex];
    const auto& idxAccessor = gltfModel_.accessors[primitive.indices];
    return idxAccessor.count * sizeof(std::uint16_t);
}

std::uint32_t GltfModelAsset::GetIndexCount(const std::uint32_t meshIndex, const std::uint32_t primitiveIndex) const
{
    const auto& primitive = gltfModel_.meshes[meshIndex].primitives[primitiveIndex];
    const auto& idxAccessor = gltfModel_.accessors[primitive.indices];
    return idxAccessor.count;
}

std::uint32_t GltfModelAsset::GetPrimitiveMaterialIndex(std::uint32_t meshIndex, std::uint32_t primitiveIndex) const
{
    return gltfModel_.meshes[meshIndex].primitives[primitiveIndex].material;
}

TextureAsset GltfModelAsset::GetTexture(const int textureIndex, const std::string& basePath) const
{
    if (textureIndex >= 0 && textureIndex < gltfModel_.images.size()) {
        TextureAsset textureAsset;
        if (const auto& image = gltfModel_.images[textureIndex]; image.uri.empty()) {
            textureAsset.width = image.width;
            textureAsset.height = image.height;
            textureAsset.channels = image.component;
            textureAsset.data = image.image;
        } else {
            TextureLoader textureLoader{""};
            textureAsset = *textureLoader.Load(basePath + image.uri);
        }

        return textureAsset;
    }

    throw std::runtime_error{"Texture index couldn't be found in the model data!"};
}

glm::mat4 GltfModelAsset::GetNodeWorldTransform(const std::uint32_t nodeIndex)
{
    if (!isTransformCalculated_) {
        nodeTransforms_.resize(gltfModel_.nodes.size());
        for (size_t i = 0; i < gltfModel_.nodes.size(); ++i) {
            nodeTransforms_[i].localTransform = GetLocalTransform(gltfModel_.nodes[i]);
        }

        const auto currentSceneIndex = gltfModel_.defaultScene > -1 ? gltfModel_.defaultScene : 0;
        for (const auto& scene = gltfModel_.scenes[currentSceneIndex]; const auto rootNode: scene.nodes) {
            ComputeWorldTransform(nodeTransforms_, static_cast<std::uint32_t>(rootNode), glm::mat4(1.0f));
        }

        isTransformCalculated_ = true;
    }

    return nodeTransforms_[nodeIndex].worldTransform;
}

MeshletData GltfModelAsset::BuildMeshlets(const std::uint32_t meshIndex,
                                          const std::uint32_t primitiveIndex,
                                          const std::size_t maxVertices,
                                          const std::size_t maxTriangles,
                                          const float coneWeight) const
{
    const auto vertices = GetVertices(meshIndex, primitiveIndex);
    const auto indices = GetIndices(meshIndex, primitiveIndex);

    const std::size_t maxMeshlets = meshopt_buildMeshletsBound(indices.size(), maxVertices, maxTriangles);

    std::vector<meshopt_Meshlet> rawMeshlets(maxMeshlets);
    std::vector<unsigned int> rawMeshletVertices(maxMeshlets * maxVertices);
    std::vector<unsigned char> rawMeshletTris(maxMeshlets * maxTriangles * 3);

    const std::size_t meshletCount =
            meshopt_buildMeshlets(rawMeshlets.data(), rawMeshletVertices.data(), rawMeshletTris.data(), indices.data(),
                                  indices.size(), &vertices[0].Position.data.x, vertices.size(),
                                  sizeof(utility::VertexPos3Uv2), maxVertices, maxTriangles, coneWeight);

    rawMeshlets.resize(meshletCount);
    const auto& lastRawMeshlet = rawMeshlets.back();
    rawMeshletVertices.resize(lastRawMeshlet.vertex_offset + lastRawMeshlet.vertex_count);
    rawMeshletTris.resize(lastRawMeshlet.triangle_offset + ((lastRawMeshlet.triangle_count * 3 + 3) & ~3u));

    MeshletData result;
    result.vertices = vertices;
    result.meshletVertices.assign(rawMeshletVertices.begin(), rawMeshletVertices.end());
    result.meshletTriangles.assign(rawMeshletTris.begin(), rawMeshletTris.end());
    result.meshlets.reserve(meshletCount);
    result.bounds.reserve(meshletCount);

    // Calculate and assign each meshlet descriptor and bounds
    for (const auto& meshlet: rawMeshlets) {
        result.meshlets.push_back(
                {meshlet.vertex_offset, meshlet.triangle_offset, meshlet.vertex_count, meshlet.triangle_count});

        const auto b = meshopt_computeMeshletBounds(
                &rawMeshletVertices[meshlet.vertex_offset], &rawMeshletTris[meshlet.triangle_offset],
                meshlet.triangle_count, &vertices[0].Position.data.x, vertices.size(), sizeof(utility::VertexPos3Uv2));

        result.bounds.push_back({{b.center[0], b.center[1], b.center[2]},
                                 b.radius,
                                 {b.cone_axis[0], b.cone_axis[1], b.cone_axis[2]},
                                 b.cone_cutoff});
    }

    return result;
}

glm::mat4 GltfModelAsset::GetLocalTransform(const tinygltf::Node& node)
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

void GltfModelAsset::ComputeWorldTransform(std::vector<NodeTransform>& nodeTransforms,
                                           const std::uint32_t nodeIndex,
                                           const glm::mat4& parentWorldMatrix)
{
    auto& nodeTransform = nodeTransforms[nodeIndex];
    nodeTransform.worldTransform = parentWorldMatrix * nodeTransform.localTransform;

    for (const auto childIndex: gltfModel_.nodes[nodeIndex].children) {
        ComputeWorldTransform(nodeTransforms, childIndex, nodeTransform.worldTransform);
    }
}

} // namespace common::asset_manager
