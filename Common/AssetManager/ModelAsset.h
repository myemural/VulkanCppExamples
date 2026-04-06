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
