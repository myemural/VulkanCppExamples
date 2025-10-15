/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    15.10.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include "ModelLoader.h"
#include "Vertex.h"
#include "glm/glm.hpp"

namespace examples::fundamentals::model_loading::gltf_multiple_meshes
{
// Vertex Attribute Layout
struct VertexPos3Uv2
{
    common::utility::Attribute<common::utility::Vec3, 0> Position; // layout(location=0) in vec3 position;
    common::utility::Attribute<common::utility::Vec2, 1> Uv;       // layout(location=1) in vec2 texCoord;
};

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};
} // namespace examples::fundamentals::model_loading::gltf_multiple_meshes

namespace common::utility
{
template<>
inline std::vector<examples::fundamentals::model_loading::gltf_multiple_meshes::VertexPos3Uv2> GltfMesh::GetVerticesAs()
{
    std::vector<examples::fundamentals::model_loading::gltf_multiple_meshes::VertexPos3Uv2> result;
    for (const auto& vertex: Vertices) {
        examples::fundamentals::model_loading::gltf_multiple_meshes::VertexPos3Uv2 current{};
        current.Position.data.X = vertex.Position.x;
        current.Position.data.Y = vertex.Position.y;
        current.Position.data.Z = vertex.Position.z;
        current.Uv.data.X = vertex.TexCoords[0].x;
        current.Uv.data.Y = vertex.TexCoords[0].y;
        result.push_back(current);
    }

    return result;
}
} // namespace common::utility
