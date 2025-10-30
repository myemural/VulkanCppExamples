/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    30.10.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include "Vertex.h"
#include "glm/glm.hpp"

namespace examples::fundamentals::multisampling::sample_shading
{
#define NUM_CUBES 10

// Vertex Attribute Layout
struct VertexPos3Uv2
{
    common::utility::Attribute<common::utility::Vec3, 0> Position; // layout(location=0) in vec3 position;
    common::utility::Attribute<common::utility::Vec2, 1> Uv;       // layout(location=1) in vec2 texCoord;
};

// Vertex Data
const std::vector vertices{
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

// Index Data
const std::vector<uint16_t> indices{
    0,  1,  2,  2,  3,  0,  // Front
    4,  5,  6,  6,  7,  4,  // Back
    8,  9,  10, 10, 11, 8,  // Left
    12, 13, 14, 14, 15, 12, // Right
    16, 17, 18, 18, 19, 16, // Top
    20, 21, 22, 22, 23, 20  // Bottom
};

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};

// Model position vectors
inline constexpr glm::vec3 modelPositions[NUM_CUBES] = {glm::vec3(0.0f, 0.0f, 0.0f),  glm::vec3(-2.1f, 0.9f, -0.7f),
                                                        glm::vec3(0.6f, 2.7f, -1.5f), glm::vec3(-1.9f, -2.5f, 1.1f),
                                                        glm::vec3(2.5f, -0.4f, 1.6f), glm::vec3(-0.8f, 1.3f, -2.7f),
                                                        glm::vec3(1.9f, 0.3f, -1.8f), glm::vec3(-2.7f, -1.5f, 2.2f),
                                                        glm::vec3(0.4f, 1.6f, -0.9f), glm::vec3(-1.4f, -0.7f, 2.6f)};
} // namespace examples::fundamentals::multisampling::sample_shading
