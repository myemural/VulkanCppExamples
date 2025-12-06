/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    28.09.2025
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
#include "VulkanHelpers.h"

namespace examples::fundamentals::pipelines_and_passes::multiple_render_passes
{
#define NUM_CUBES 10

// Vertex Data for Cube
inline const std::vector cubeVertices = common::utility::CreateCubeVertices<common::utility::VertexPos3Uv2>(1.0f);

// Index Data for Cube
inline const std::vector cubeIndices = common::utility::CreateCubeIndices();

// Vertex Data for Plane (XY)
inline const std::vector planeVertices = common::utility::CreateQuadVerticesXY<common::utility::VertexPos3Uv2>(1.0f);

// Index Data for Plane (XY)
inline const std::vector<std::uint16_t> planeIndices = common::utility::CreateQuadIndices();

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};

// Model Matrix (for Uniform Buffer)
struct UniformBufferObject
{
    float time;
    float speed;
};

// Model position vectors
inline const std::vector<glm::vec3> modelPositions = common::vulkan_framework::GenerateRandomPositions(
        NUM_CUBES, glm::vec3(-3.0f, -2.5f, -4.0f), glm::vec3(3.0f, 2.5f, -1.0f), 2.0f);
} // namespace examples::fundamentals::pipelines_and_passes::multiple_render_passes
