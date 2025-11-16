/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    11.09.2025
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

namespace examples::fundamentals::drawing_3d::face_culling
{

#define NUM_CUBES 10

// Vertex Data for Cube
// clang-format off
inline const std::vector<common::utility::VertexPos3Uv2> vertices {
    // Front face
    {glm::vec3{-0.5f, -0.5f, 0.5f}, glm::vec2{0.0f, 0.0f}}, // 0
    {glm::vec3{0.5f, -0.5f, 0.5f}, glm::vec2{1.0f, 0.0f}},  // 1
    {glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec2{1.0f, 1.0f}},   // 2
    {glm::vec3{-0.5f, 0.5f, 0.5f}, glm::vec2{0.0f, 1.0f}},  // 3

    // Back face
    {glm::vec3{-0.5f, -0.5f, -0.5f}, glm::vec2{1.0f, 0.0f}}, // 4
    {glm::vec3{0.5f, -0.5f, -0.5f}, glm::vec2{0.0f, 0.0f}},  // 5
    {glm::vec3{0.5f, 0.5f, -0.5f}, glm::vec2{0.0f, 1.0f}},   // 6
    {glm::vec3{-0.5f, 0.5f, -0.5f}, glm::vec2{1.0f, 1.0f}},  // 7

    // Left face
    {glm::vec3{-0.5f, -0.5f, -0.5f}, glm::vec2{0.0f, 0.0f}}, // 8
    {glm::vec3{-0.5f, -0.5f, 0.5f}, glm::vec2{1.0f, 0.0f}},  // 9
    {glm::vec3{-0.5f, 0.5f, 0.5f}, glm::vec2{1.0f, 1.0f}},   // 10
    {glm::vec3{-0.5f, 0.5f, -0.5f}, glm::vec2{0.0f, 1.0f}},  // 11

    // Right face
    {glm::vec3{0.5f, -0.5f, -0.5f}, glm::vec2{1.0f, 0.0f}}, // 12
    {glm::vec3{0.5f, -0.5f, 0.5f}, glm::vec2{0.0f, 0.0f}},  // 13
    {glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec2{0.0f, 1.0f}},   // 14
    {glm::vec3{0.5f, 0.5f, -0.5f}, glm::vec2{1.0f, 1.0f}},  // 15

    // Top face
    {glm::vec3{-0.5f, 0.5f, 0.5f}, glm::vec2{0.0f, 0.0f}},  // 16
    {glm::vec3{0.5f, 0.5f, 0.5f}, glm::vec2{1.0f, 0.0f}},   // 17
    {glm::vec3{0.5f, 0.5f, -0.5f}, glm::vec2{1.0f, 1.0f}},  // 18
    {glm::vec3{-0.5f, 0.5f, -0.5f}, glm::vec2{0.0f, 1.0f}}, // 19

    // Bottom face
    {glm::vec3{-0.5f, -0.5f, 0.5f}, glm::vec2{0.0f, 0.0f}}, // 20
    {glm::vec3{0.5f, -0.5f, 0.5f}, glm::vec2{1.0f, 0.0f}},  // 21
    {glm::vec3{0.5f, -0.5f, -0.5f}, glm::vec2{1.0f, 1.0f}}, // 22
    {glm::vec3{-0.5f, -0.5f, -0.5f}, glm::vec2{0.0f, 1.0f}} // 23
};
// clang-format on

// Index Data for Cube
inline const std::vector indices = common::utility::CreateCubeIndices();

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
} // namespace examples::fundamentals::drawing_3d::face_culling
