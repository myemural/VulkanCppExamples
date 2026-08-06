/**
 * @file    ModelAsset.h
 * @brief   Contains 3D model asset data classes.
 * @author  Mustafa Yemural (myemural)
 * @date    30.03.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <utility>

#include <tiny_gltf.h>
#include <glm/glm.hpp>

#include "CoreDefines.h"
#include "TextureLoader.h"
#include "Vertex.h"

namespace common::asset_manager
{

struct COMMON_API NodeTransform
{
    glm::mat4 localTransform;
    glm::mat4 worldTransform;
};

struct COMMON_API MeshletDescriptor
{
    std::uint32_t vertexOffset;
    std::uint32_t triangleOffset;
    std::uint32_t vertexCount;
    std::uint32_t triangleCount;
};

struct COMMON_API MeshletBounds
{
    glm::vec3 center;
    float radius;
    glm::vec3 coneAxis;
    float coneCutoff;
};

struct COMMON_API MeshletData
{
    std::vector<utility::VertexPos3Uv2> vertices;
    std::vector<std::uint32_t> meshletVertices;
    std::vector<std::uint8_t> meshletTriangles; // Packed, needs to expand
    std::vector<MeshletDescriptor> meshlets;
    std::vector<MeshletBounds> bounds;
};

struct COMMON_API LodRange
{
    std::uint32_t meshletOffset;
    std::uint32_t meshletCount;
};

struct COMMON_API MeshletLodData
{
    std::vector<utility::VertexPos3Uv2> vertices;
    std::vector<std::uint32_t> meshletVertices;
    std::vector<std::uint32_t> meshletTriangles; // Packed, needs to expand
    std::vector<MeshletDescriptor> meshlets;
    std::vector<MeshletBounds> bounds;
    std::vector<LodRange> lodRanges;             // Specifies LOD ranges per LOD level
};

class COMMON_API GltfModelAsset
{
public:
    explicit GltfModelAsset(tinygltf::Model gltfModel) : gltfModel_{std::move(gltfModel)} {}

    [[nodiscard]] tinygltf::Model& GetModel() { return gltfModel_; }

    [[nodiscard]] const tinygltf::Model& GetModel() const { return gltfModel_; }

    [[nodiscard]] std::vector<utility::VertexPos3Uv2> GetVertices(std::uint32_t meshIndex,
                                                                  std::uint32_t primitiveIndex) const;

    [[nodiscard]] std::vector<std::uint16_t> GetIndices(std::uint32_t meshIndex, std::uint32_t primitiveIndex) const;

    [[nodiscard]] std::uint32_t GetVertexBufferSize(std::uint32_t meshIndex, std::uint32_t primitiveIndex) const;

    [[nodiscard]] std::uint32_t GetIndexBufferSize(std::uint32_t meshIndex, std::uint32_t primitiveIndex) const;

    [[nodiscard]] std::uint32_t GetIndexCount(std::uint32_t meshIndex, std::uint32_t primitiveIndex) const;

    [[nodiscard]] std::uint32_t GetPrimitiveMaterialIndex(std::uint32_t meshIndex, std::uint32_t primitiveIndex) const;

    [[nodiscard]] TextureAsset GetTexture(int textureIndex, const std::string& basePath = "") const;

    [[nodiscard]] glm::mat4 GetNodeWorldTransform(std::uint32_t nodeIndex);

    [[nodiscard]] MeshletData BuildMeshlets(std::uint32_t meshIndex,
                                            std::uint32_t primitiveIndex,
                                            std::size_t maxVertices = 64,
                                            std::size_t maxTriangles = 124,
                                            float coneWeight = 0.5f) const;

    /**
     * @brief Builds several LOD levels of the same primitive and returns them pre-combined and ready to upload to a
     * single set of GPU buffers.
     * @param meshIndex Index of the mesh in glTF model.
     * @param primitiveIndex Index of the primitive within the mesh.
     * @param lodTargetRatios Vector of target triangle counts for each LOD level.
     * @param simplifyTargetError Error tolerance for the simplification process.
     * @param maxVertices Maximum number of vertices per meshlet.
     * @param maxTriangles Maximum number of triangles per meshlet.
     * @param coneWeight Weight for the cone culling algorithm.
     * @return Meshlet data with LOD information.
     */
    [[nodiscard]] MeshletLodData BuildMeshletWithLod(std::uint32_t meshIndex,
                                                     std::uint32_t primitiveIndex,
                                                     const std::vector<float>& lodTargetRatios,
                                                     float simplifyTargetError = 0.05f,
                                                     std::size_t maxVertices = 64,
                                                     std::size_t maxTriangles = 124,
                                                     float coneWeight = 0.5f) const;

private:
    static glm::mat4 GetLocalTransform(const tinygltf::Node& node);

    void ComputeWorldTransform(std::vector<NodeTransform>& nodeTransforms,
                               std::uint32_t nodeIndex,
                               const glm::mat4& parentWorldMatrix);

    tinygltf::Model gltfModel_;
    std::vector<NodeTransform> nodeTransforms_;
    bool isTransformCalculated_ = false;
};

} // namespace common::asset_manager
