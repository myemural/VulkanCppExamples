/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    2.11.2025
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

namespace examples::fundamentals::multisampling::explicit_resolving
{
#define NUM_SPHERE 10

// Vertex Data for Sphere
inline const std::vector sphereVertices = common::utility::CreateSphereVertices<common::utility::VertexPos3Uv2>(0.5f, 32, 16);

// Index Data for Sphere
inline const std::vector<uint16_t> sphereIndices = common::utility::CreateSphereIndices(32, 16);

// Vertex Data for Quad (XY)
inline const std::vector planeVertices = common::utility::CreateQuadVerticesXY<common::utility::VertexPos3Uv2>(1.0f);

// Index Data for Quad (XY)
inline const std::vector<uint16_t> planeIndices = common::utility::CreateQuadIndices();

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};

// Model position vectors
inline constexpr glm::vec3 modelPositions[NUM_SPHERE] = {glm::vec3(0.0f, 0.0f, 0.0f),  glm::vec3(-2.1f, 0.9f, -0.7f),
                                                         glm::vec3(0.6f, 2.7f, -1.5f), glm::vec3(-1.9f, -2.5f, 1.1f),
                                                         glm::vec3(2.5f, -0.4f, 1.6f), glm::vec3(-0.8f, 1.3f, -2.7f),
                                                         glm::vec3(1.9f, 0.3f, -1.8f), glm::vec3(-2.7f, -1.5f, 2.2f),
                                                         glm::vec3(0.4f, 1.6f, -0.9f), glm::vec3(-1.4f, -0.7f, 2.6f)};
} // namespace examples::fundamentals::multisampling::explicit_resolving
