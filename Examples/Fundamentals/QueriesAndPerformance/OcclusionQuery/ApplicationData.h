/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    21.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "PrimitiveUtils.h"
#include "Vertex.h"

namespace examples::fundamentals::queries_and_performance::occlusion_query
{

// Vertex Data for Cube
inline const std::vector cubeVertices = common::utility::CreateCubeVertices<common::utility::VertexPos3Uv2>(2.5f);

// Vertex Data for Sphere
inline const std::vector sphereVertices =
        common::utility::CreateSphereVertices<common::utility::VertexPos3Uv2>(0.2f, 32, 16);

// Index Data for Cube
inline const std::vector<uint16_t> cubeIndices = common::utility::CreateCubeIndices();

// Index Data for Sphere
inline const std::vector<uint16_t> sphereIndices = common::utility::CreateSphereIndices(32, 16);

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};

} // namespace examples::fundamentals::queries_and_performance::occlusion_query
