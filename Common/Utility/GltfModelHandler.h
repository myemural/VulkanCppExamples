/**
 * @file    GltfModelHandler.h
 * @brief   Contains implementation of the handler type for glTF models and its helper types.
 * @author  Mustafa Yemural (myemural)
 * @date    12.10.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "CoreDefines.h"
#include "TextureHandler.h"
#include "Vertex.h"

namespace common::utility
{

struct COMMON_API GltfPrimitiveAttrib
{
    std::uint32_t vertexCount;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> tangents;
    std::vector<glm::vec2> texCoords0;
    std::vector<glm::vec2> texCoords1;
    std::vector<glm::vec4> colors0;
    std::vector<glm::vec4> colors1;
    /// TODO: Joints and weights will be added later
};

struct COMMON_API GltfMaterial
{
    struct PbrMetallicRoughness
    {
        int baseColorTextureIndex = -1;
    };

    std::string name;
    PbrMetallicRoughness pbrMetallicRoughness;
};

struct COMMON_API GltfMesh
{
    std::string name;
    GltfPrimitiveAttrib attributes;
    std::vector<std::uint16_t> indices;
    int materialIndex = -1;

    template<typename VertexType>
    std::vector<VertexType> GetVerticesAs();

    [[nodiscard]] std::string GetVertexBufferName() const { return name + "_VertexBuffer"; }

    [[nodiscard]] std::string GetIndexBufferName() const { return name + "_IndexBuffer"; }
};

struct COMMON_API GltfNode
{
    std::uint32_t parentIndex = UINT32_MAX;
    std::vector<std::uint32_t> childIndices;
    std::uint32_t meshIndex = UINT32_MAX;
    std::uint32_t cameraIndex = UINT32_MAX;
    glm::mat4 localTransform = glm::mat4(1.0f);
    glm::mat4 worldTransform = glm::mat4(1.0f);
};

enum class GltfCameraType
{
    PERSPECTIVE,
    ORTHOGRAPHIC
};

struct COMMON_API GltfCamera
{
    std::string name;
    GltfCameraType type;

    struct Perspective
    {
        float aspectRatio;
        float fov;
        float near;
        float far;
    };

    struct Orthographic
    {
        float aspectRatio;
        float size;
        float near;
        float far;
    };

    Perspective perspectiveFeatures;
    Orthographic orthographicFeatures;
};

struct COMMON_API GltfModelHandler
{
    std::string name;
    std::uint32_t currentSceneIndex = UINT32_MAX;
    std::vector<GltfCamera> cameras;
    std::vector<GltfNode> nodes;
    std::vector<GltfMesh> meshes;
    std::vector<GltfMaterial> materials;
    std::vector<TextureHandler> textures;
};

template<>
inline std::vector<VertexPos3> GltfMesh::GetVerticesAs()
{
    std::vector<VertexPos3> result{attributes.positions.size()};

    for (auto i = 0U; i < attributes.positions.size(); ++i) {
        VertexPos3 current{};
        current.Position.data = attributes.positions[i];
        result[i] = current;
    }

    return result;
}

template<>
inline std::vector<VertexPos3Uv2> GltfMesh::GetVerticesAs()
{
    std::vector<VertexPos3Uv2> result{attributes.positions.size()};

    for (auto i = 0U; i < attributes.positions.size(); ++i) {
        VertexPos3Uv2 current{};
        current.Position.data = attributes.positions[i];
        current.Uv.data.x = attributes.texCoords0[i].x;
        current.Uv.data.y = attributes.texCoords0[i].y;
        result[i] = current;
    }

    return result;
}

} // namespace common::utility
