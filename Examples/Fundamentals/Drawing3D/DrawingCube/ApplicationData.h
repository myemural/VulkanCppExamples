/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    08.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include <glm/mat4x4.hpp>

#include "Vertex.h"

namespace examples::fundamentals::drawing_3d::drawing_cube
{

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
// clang-format off
inline const std::vector<uint16_t> indices {
    0,  1,  2,  2,  3,  0,  // Front
    4,  5,  6,  6,  7,  4,  // Back
    8,  9,  10, 10, 11, 8,  // Left
    12, 13, 14, 14, 15, 12, // Right
    16, 17, 18, 18, 19, 16, // Top
    20, 21, 22, 22, 23, 20  // Bottom
};
// clang-format on

// MVP Matrices (for Uniform Buffer)
struct MvpData
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};
} // namespace examples::fundamentals::drawing_3d::drawing_cube
