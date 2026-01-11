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

#include "MathUtils.h"
#include "PrimitiveUtils.h"
#include "Vertex.h"

namespace examples::fundamentals::multisampling::explicit_resolving
{
#define NUM_SPHERES 10

// Vertex Data for Sphere
inline const std::vector sphereVertices =
        common::utility::CreateSphereVertices<common::utility::VertexPos3Uv2>(0.5f, 32, 16);

// Index Data for Sphere
inline const std::vector<std::uint16_t> sphereIndices = common::utility::CreateSphereIndices(32, 16);

// Vertex Data for Quad (XY)
inline const std::vector planeVertices = common::utility::CreateQuadVerticesXY<common::utility::VertexPos3Uv2>(1.0f);

// Index Data for Quad (XY)
inline const std::vector<std::uint16_t> planeIndices = common::utility::CreateQuadIndices();

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};

// Model position vectors
inline const std::vector<glm::vec3> modelPositions = common::utility::GenerateRandomPositions(
        NUM_SPHERES, glm::vec3(-3.0f, -2.5f, -4.0f), glm::vec3(3.0f, 2.5f, -1.0f), 2.0f);
} // namespace examples::fundamentals::multisampling::explicit_resolving
