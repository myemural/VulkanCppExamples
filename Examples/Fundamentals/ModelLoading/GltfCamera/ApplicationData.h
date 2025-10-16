/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    16.10.2025
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

namespace examples::fundamentals::model_loading::gltf_camera
{
// Vertex Attribute Layout
struct VertexPos3
{
    common::utility::Attribute<common::utility::Vec3, 0> Position; // layout(location=0) in vec3 position;
};

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};
} // namespace examples::fundamentals::model_loading::gltf_camera

namespace common::utility
{
template<>
inline std::vector<examples::fundamentals::model_loading::gltf_camera::VertexPos3> GltfMesh::GetVerticesAs()
{
    std::vector<examples::fundamentals::model_loading::gltf_camera::VertexPos3> result;
    for (const auto& vertex: Vertices) {
        examples::fundamentals::model_loading::gltf_camera::VertexPos3 current{};
        current.Position.data.X = vertex.Position.x;
        current.Position.data.Y = vertex.Position.y;
        current.Position.data.Z = vertex.Position.z;
        result.push_back(current);
    }

    return result;
}
} // namespace common::utility
