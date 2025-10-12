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

#include "ModelLoader.h"
#include "Vertex.h"
#include "glm/glm.hpp"

namespace examples::fundamentals::model_loading::test
{
#define NUM_OBJECTS 10

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

// Model position vectors
inline constexpr glm::vec3 modelPositions[NUM_OBJECTS] = {
    glm::vec3(0.0f, 0.0f, 0.0f),      glm::vec3(-1.05f, 0.45f, -0.35f), glm::vec3(0.3f, 1.35f, -0.75f),
    glm::vec3(-0.95f, -1.25f, 0.55f), glm::vec3(1.25f, -0.2f, 0.8f),    glm::vec3(-0.4f, 0.65f, -1.35f),
    glm::vec3(0.95f, 0.15f, -0.9f),   glm::vec3(-1.35f, -0.75f, 1.1f),  glm::vec3(0.2f, 0.8f, -0.45f),
    glm::vec3(-0.7f, -0.35f, 1.3f)};
} // namespace examples::fundamentals::model_loading::test

namespace common::utility
{
template<>
inline std::vector<examples::fundamentals::model_loading::test::VertexPos3Uv2> GltfMesh::GetVerticesAs()
{
    std::vector<examples::fundamentals::model_loading::test::VertexPos3Uv2> result;
    for (const auto& vertex: Vertices) {
        examples::fundamentals::model_loading::test::VertexPos3Uv2 current{};
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
