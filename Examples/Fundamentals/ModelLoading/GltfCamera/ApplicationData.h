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

#include <glm/mat4x4.hpp>

#include "ModelLoader.h"
#include "Vertex.h"

namespace examples::fundamentals::model_loading::gltf_camera
{

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};
} // namespace examples::fundamentals::model_loading::gltf_camera

namespace common::utility
{
template<>
inline std::vector<VertexPos3> GltfMesh::GetVerticesAs()
{
    std::vector<VertexPos3> result;
    for (const auto& vertex: Vertices) {
        result.emplace_back(VertexPos3{vertex.Position});
    }

    return result;
}
} // namespace common::utility
