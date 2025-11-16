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

#include <glm/mat4x4.hpp>

#include "ModelLoader.h"
#include "Vertex.h"

namespace examples::fundamentals::model_loading::gltf_multiple_meshes
{

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};
} // namespace examples::fundamentals::model_loading::gltf_multiple_meshes

namespace common::utility
{
template<>
inline std::vector<VertexPos3Uv2> GltfMesh::GetVerticesAs()
{
    std::vector<VertexPos3Uv2> result;
    for (const auto& vertex: Vertices) {
        VertexPos3Uv2 current{};
        current.Position.data = vertex.Position;
        current.Uv.data.x = vertex.TexCoords[0].x;
        current.Uv.data.y = vertex.TexCoords[0].y;
        result.push_back(current);
    }

    return result;
}
} // namespace common::utility
