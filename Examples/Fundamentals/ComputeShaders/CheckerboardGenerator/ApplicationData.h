/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    9.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "PrimitiveUtils.h"
#include "Vertex.h"

namespace examples::fundamentals::compute_shaders::checkerboard_generator
{

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16
#define NUM_CUBES 5
#define NUM_SPHERES 5

// Vertex Attribute Layout
struct VertexPos3Uv2
{
    common::utility::Attribute<common::utility::Vec3, 0> Position; // layout(location=0) in vec3 position;
    common::utility::Attribute<common::utility::Vec2, 1> Uv;       // layout(location=1) in vec2 texCoord;
};

// Vertex Data for Cubes
const std::vector cubeVertices = common::utility::CreateCubeVertices<VertexPos3Uv2>(1.0f);

// Vertex Data for Spheres
const std::vector sphereVertices = common::utility::CreateSphereVertices<VertexPos3Uv2>(0.5f, 32, 16);

// Index Data for Quad
const std::vector<uint16_t> cubeIndices = common::utility::CreateCubeIndices();

// Index Data for Sphere
const std::vector<uint16_t> sphereIndices = common::utility::CreateSphereIndices(32, 16);

struct MvpData
{
    glm::mat4 mvpMatrix;
};

// Cube position vectors
inline constexpr glm::vec3 cubePositions[NUM_CUBES] = {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-2.5f, 1.2f, -0.6f),
                                                       glm::vec3(1.2f, 2.6f, -1.8f), glm::vec3(-1.8f, -2.4f, 1.4f),
                                                       glm::vec3(2.8f, -0.5f, 1.9f)};

// Sphere position vectors
inline constexpr glm::vec3 spherePositions[NUM_SPHERES] = {glm::vec3(1.8f, -0.8f, 2.6f), glm::vec3(-2.6f, 1.8f, 1.5f),
                                                           glm::vec3(0.6f, 3.0f, -2.8f), glm::vec3(-1.2f, -3.0f, -0.5f),
                                                           glm::vec3(2.4f, -0.1f, -2.2f)};

} // namespace examples::fundamentals::compute_shaders::checkerboard_generator
