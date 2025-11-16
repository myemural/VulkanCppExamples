/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    12.09.2025
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

namespace examples::fundamentals::drawing_3d::instanced_rendering
{
#define NUM_CUBES 20

// Vertex Data for Cube
inline const std::vector vertices = common::utility::CreateCubeVertices<common::utility::VertexPos3Uv2>(1.0f);

// Index Data for Cube
inline const std::vector indices = common::utility::CreateCubeIndices();

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};

// Model position vectors
inline constexpr glm::vec3 modelPositions[NUM_CUBES] = {
    glm::vec3(0.0f, 0.0f, 0.0f),   glm::vec3(-4.0f, 1.5f, -5.0f), glm::vec3(5.0f, -1.2f, 3.0f),
    glm::vec3(-3.0f, 4.0f, -2.5f), glm::vec3(6.0f, -3.5f, 5.0f),  glm::vec3(-1.5f, -5.0f, 2.5f),
    glm::vec3(-5.0f, 1.0f, -4.0f), glm::vec3(2.0f, 4.5f, -3.5f),  glm::vec3(4.5f, -3.0f, 5.5f),
    glm::vec3(-3.5f, 5.0f, -1.0f), glm::vec3(3.5f, -2.0f, 4.5f),  glm::vec3(-5.5f, -1.5f, 2.0f),
    glm::vec3(1.0f, 3.0f, -5.0f),  glm::vec3(5.5f, -3.0f, -2.5f), glm::vec3(-2.5f, 5.5f, 1.0f),
    glm::vec3(5.0f, -4.0f, -5.0f), glm::vec3(-4.0f, 1.5f, 4.0f),  glm::vec3(3.0f, 2.5f, -3.0f),
    glm::vec3(-1.5f, -5.5f, 5.0f), glm::vec3(2.5f, 4.5f, -0.5f)};
} // namespace examples::fundamentals::drawing_3d::instanced_rendering
