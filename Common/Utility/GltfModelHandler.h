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
    std::uint32_t VertexCount;
    std::vector<glm::vec3> Positions;
    std::vector<glm::vec3> Normals;
    std::vector<glm::vec4> Tangents;
    std::vector<glm::vec2> TexCoords0;
    std::vector<glm::vec2> TexCoords1;
    std::vector<glm::vec4> Colors0;
    std::vector<glm::vec4> Colors1;
    /// TODO: Joints and weights will be added later
};

struct COMMON_API GltfMaterial
{
    std::string Name;
    struct PbrMetallicRoughness
    {
        int BaseColorTextureIndex = -1;
    } PbrMetallicRoughness;
};

struct COMMON_API GltfMesh
{
    std::string Name;
    GltfPrimitiveAttrib Attributes;
    std::vector<std::uint16_t> Indices;
    int MaterialIndex = -1;

    template<typename VertexType>
    std::vector<VertexType> GetVerticesAs();

    [[nodiscard]] std::string GetVertexBufferName() const
    {
        return Name + "_VertexBuffer";
    }

    [[nodiscard]] std::string GetIndexBufferName() const
    {
        return Name + "_IndexBuffer";
    }
};

struct COMMON_API GltfNode
{
    std::uint32_t ParentIndex = UINT32_MAX;
    std::vector<std::uint32_t> ChildIndices;
    std::uint32_t MeshIndex = UINT32_MAX;
    std::uint32_t CameraIndex = UINT32_MAX;
    glm::mat4 LocalTransform = glm::mat4(1.0f);
    glm::mat4 WorldTransform = glm::mat4(1.0f);
};

enum class GltfCameraType
{
    PERSPECTIVE,
    ORTHOGRAPHIC
};

struct COMMON_API GltfCamera
{
    std::string Name;
    GltfCameraType Type;

    struct Perspective
    {
        float AspectRatio;
        float Fov;
        float Near;
        float Far;
    } PerspectiveFeatures;

    struct Orthographic
    {
        float AspectRatio;
        float Size;
        float Near;
        float Far;
    } OrthographicFeatures;
};

struct COMMON_API GltfModelHandler
{
    std::string Name;
    std::uint32_t CurrentSceneIndex = UINT32_MAX;
    std::vector<GltfCamera> Cameras;
    std::vector<GltfNode> Nodes;
    std::vector<GltfMesh> Meshes;
    std::vector<GltfMaterial> Materials;
    std::vector<TextureHandler> Textures;
};

template<>
inline std::vector<VertexPos3> GltfMesh::GetVerticesAs()
{
    std::vector<VertexPos3> result{Attributes.Positions.size()};

    for (auto i = 0U; i < Attributes.Positions.size(); ++i) {
        VertexPos3 current{};
        current.Position.data = Attributes.Positions[i];
        result[i] = current;
    }

    return result;
}

template<>
inline std::vector<VertexPos3Uv2> GltfMesh::GetVerticesAs()
{
    std::vector<VertexPos3Uv2> result{Attributes.Positions.size()};

    for (auto i = 0U; i < Attributes.Positions.size(); ++i) {
        VertexPos3Uv2 current{};
        current.Position.data = Attributes.Positions[i];
        current.Uv.data.x = Attributes.TexCoords0[i].x;
        current.Uv.data.y = Attributes.TexCoords0[i].y;
        result[i] = current;
    }

    return result;
}

} // namespace common::utility