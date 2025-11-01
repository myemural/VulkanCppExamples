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

#include "CoreDefines.h"
#include "TextureHandler.h"

namespace common::utility
{

struct COMMON_API GltfPrimitiveAttrib
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
    std::vector<GltfPrimitiveAttrib> Vertices;
    std::vector<uint16_t> Indices;
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

} // namespace common::utility