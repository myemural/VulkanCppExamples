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

#include "Vertex.h"
#include "glm/glm.hpp"

namespace examples::fundamentals::drawing_3d::depth_testing_operations
{
#define NUM_CUBES 2
#define NUM_PLANES 2

// Vertex Attribute Layout
struct VertexPos3Uv2
{
    common::utility::Attribute<common::utility::Vec3, 0> Position; // layout(location=0) in vec3 position;
    common::utility::Attribute<common::utility::Vec2, 1> Uv;       // layout(location=1) in vec2 texCoord;
};

// Vertex Data for Cube
const std::vector cubeVertices{
    // Front face
    VertexPos3Uv2{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}}, // 0
    VertexPos3Uv2{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},  // 1
    VertexPos3Uv2{{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},   // 2
    VertexPos3Uv2{{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},  // 3

    // Back face
    VertexPos3Uv2{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}}, // 4
    VertexPos3Uv2{{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},  // 5
    VertexPos3Uv2{{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},   // 6
    VertexPos3Uv2{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},  // 7

    // Left face
    VertexPos3Uv2{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}}, // 8
    VertexPos3Uv2{{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},  // 9
    VertexPos3Uv2{{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},   // 10
    VertexPos3Uv2{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},  // 11

    // Right face
    VertexPos3Uv2{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}}, // 12
    VertexPos3Uv2{{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},  // 13
    VertexPos3Uv2{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},   // 14
    VertexPos3Uv2{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},  // 15

    // Top face
    VertexPos3Uv2{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},  // 16
    VertexPos3Uv2{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},   // 17
    VertexPos3Uv2{{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},  // 18
    VertexPos3Uv2{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}}, // 19

    // Bottom face
    VertexPos3Uv2{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}}, // 20
    VertexPos3Uv2{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},  // 21
    VertexPos3Uv2{{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}}, // 22
    VertexPos3Uv2{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}} // 23
};

// Vertex Data for Plane (XZ)
const std::vector planeVertices{
    VertexPos3Uv2{{-1.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}, // 0
    VertexPos3Uv2{{1.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},  // 1
    VertexPos3Uv2{{1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // 2
    VertexPos3Uv2{{-1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},  // 3
};

// Index Data for Cube
const std::vector<uint16_t> cubeIndices{
    0,  1,  2,  2,  3,  0,  // Front
    4,  5,  6,  6,  7,  4,  // Back
    8,  9,  10, 10, 11, 8,  // Left
    12, 13, 14, 14, 15, 12, // Right
    16, 17, 18, 18, 19, 16, // Top
    20, 21, 22, 22, 23, 20  // Bottom
};

// Index Data for Plane
const std::vector<uint16_t> planeIndices{
    0, 1, 2, // First triangle
    2, 3, 0  // Second triangle
};

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
