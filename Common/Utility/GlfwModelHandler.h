/**
 * @file    GlfwModelHandler.h
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

#include "TextureHandler.h"

namespace common::utility
{

struct GltfPrimitiveAttrib
{
    glm::vec3 Position;
    glm::vec3 Normal;
    std::vector<glm::vec2> TexCoords;
    /// TODO: These values will be added later.
    // TANGENT
    // COLOR_n
    // JOINTS_n
    // WEIGHTS_n
};

struct GltfMaterial
{
    std::string Name;
    struct PbrMetallicRoughness
    {
        int BaseColorTextureIndex = -1;
    } PbrMetallicRoughness;

    std::string GetImageName() const
    {
        return Name + "_Image";
    }

    std::string GetImageViewName() const
    {
        return Name + "_ImageView";
    }
};

struct GltfMesh
{
    std::string Name;
    std::vector<GltfPrimitiveAttrib> Vertices;
    std::vector<uint16_t> Indices;
    int MaterialIndex = -1;

    template<typename VertexType>
    std::vector<VertexType> GetVerticesAs();

    std::string GetVertexBufferName() const
    {
        return Name + "_VertexBuffer";
    }

    std::string GetIndexBufferName() const
    {
        return Name + "_IndexBuffer";
    }
};

struct GltfNode
{
    std::uint32_t ParentIndex = UINT32_MAX;
    std::vector<std::uint32_t> ChildIndices;
    std::uint32_t MeshIndex = UINT32_MAX;
    glm::mat4 LocalTransform = glm::mat4(1.0f);
    glm::mat4 WorldTransform = glm::mat4(1.0f);
};

struct GltfModelHandler
{
    std::string Name;
    std::vector<GltfNode> Nodes;
    std::vector<GltfMesh> Meshes;
    std::vector<GltfMaterial> Materials;
    std::vector<TextureHandler> Textures;
};

} // namespace common::utility