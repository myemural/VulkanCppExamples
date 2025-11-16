/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    27.09.2025
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

namespace examples::fundamentals::pipelines_and_passes::multiple_subpasses
{
#define NUM_CUBES 4

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
inline constexpr glm::vec3 modelPositions[NUM_CUBES] = {glm::vec3(-1.2f, 0.0f, -1.0f), glm::vec3(-1.2f, 0.0f, -7.0f),
                                                        glm::vec3(-1.2f, 0.0f, -13.0f), glm::vec3(1.0f, 0.0f, 0.0f)};
} // namespace examples::fundamentals::pipelines_and_passes::multiple_subpasses
