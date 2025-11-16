/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    16.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include <glm/mat4x4.hpp>

#include "PrimitiveUtils.h"
#include "Vertex.h"

namespace examples::fundamentals::drawing_3d::depth_testing_operations
{
#define NUM_CUBES 2
#define NUM_PLANES 2

// Vertex Data for Cube
inline const std::vector cubeVertices = common::utility::CreateCubeVertices<common::utility::VertexPos3Uv2>(1.0f);

// Index Data for Cube
inline const std::vector cubeIndices = common::utility::CreateCubeIndices();

// Vertex Data for Quad (XZ)
inline const std::vector planeVertices = common::utility::CreateQuadVerticesXZ<common::utility::VertexPos3Uv2>(1.0f);

// Index Data for Quad (XZ)
inline const std::vector<uint16_t> planeIndices = common::utility::CreateQuadIndices();

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};

// Cube position vectors
inline constexpr glm::vec3 cubePositions[NUM_CUBES] = {
    glm::vec3(1.5f, 0.5001f, 0.0f), // Cube 1 position
    glm::vec3(0.0f, 0.5001f, 0.5f)  // Cube 2 position
};

// Plane position vectors
inline constexpr glm::vec3 planePositions[NUM_PLANES] = {
    glm::vec3(0.0f, 0.0f, 0.0f), // Bottom plane position
    glm::vec3(0.0f, 1.0f, -4.0f) // Rear plane position
};
} // namespace examples::fundamentals::drawing_3d::depth_testing_operations
