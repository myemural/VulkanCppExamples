/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    12.10.2025
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

namespace examples::fundamentals::model_loading::gltf_mesh_wireframe
{
#define NUM_OBJECTS 10

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};

// Model position vectors
inline constexpr glm::vec3 modelPositions[NUM_OBJECTS] = {
    glm::vec3(0.0f, 0.0f, 0.0f),      glm::vec3(-1.05f, 0.45f, -0.35f), glm::vec3(0.3f, 1.35f, -0.75f),
    glm::vec3(-0.95f, -1.25f, 0.55f), glm::vec3(1.25f, -0.2f, 0.8f),    glm::vec3(-0.4f, 0.65f, -1.35f),
    glm::vec3(0.95f, 0.15f, -0.9f),   glm::vec3(-1.35f, -0.75f, 1.1f),  glm::vec3(0.2f, 0.8f, -0.45f),
    glm::vec3(-0.7f, -0.35f, 1.3f)};
} // namespace examples::fundamentals::model_loading::gltf_mesh_wireframe

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
