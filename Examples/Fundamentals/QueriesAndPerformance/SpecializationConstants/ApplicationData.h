/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    29.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include "MathUtils.h"
#include "PrimitiveUtils.h"
#include "Vertex.h"

namespace examples::fundamentals::queries_and_performance::specialization_constants
{
#define MAX_NUM_OBJECTS 600
#define PER_OBJECT_COUNT 150
#define PRIMITIVE_TYPE_COUNT 4

// Vertex Data for Cube
inline const std::vector cubeVertices = common::utility::CreateCubeVertices<common::utility::VertexPos3Uv2>(1.0f);

// Vertex Data for Sphere
inline const std::vector sphereVertices =
        common::utility::CreateSphereVertices<common::utility::VertexPos3Uv2>(0.5f, 32, 16);

// Vertex Data for Cone
inline const std::vector coneVertices =
        common::utility::CreateConeVertices<common::utility::VertexPos3Uv2>(0.5f, 1.0f, 32, 16);

// Vertex Data for Cylinder
inline const std::vector cylinderVertices =
        common::utility::CreateCylinderVertices<common::utility::VertexPos3Uv2>(0.5f, 1.5f, 32, 16);

// Index Data for Cube
inline const std::vector<std::uint16_t> cubeIndices = common::utility::CreateCubeIndices();

// Index Data for Sphere
inline const std::vector<std::uint16_t> sphereIndices = common::utility::CreateSphereIndices(32, 16);

// Index Data for Cone
inline const std::vector<std::uint16_t> coneIndices = common::utility::CreateConeIndices(32, 16);

// Index Data for Cylinder
inline const std::vector<std::uint16_t> cylinderIndices = common::utility::CreateCylinderIndices(32, 16);

struct ObjectUbo
{
    glm::mat4 mvpMatrix;
};

struct BufferAllocInfo
{
    VkDeviceSize vertexOffset;
    std::uint32_t vertexSize;
    VkDeviceSize indexOffset;
    std::uint32_t indexSize;
    std::uint32_t indexCount;
};

struct PrimitiveData
{
    std::vector<common::utility::VertexPos3Uv2> vertices;
    std::vector<std::uint16_t> indices;
    std::uint32_t drawCount;
};

enum ShaderColoringMode
{
    FLAT_COLOR = 0,
    TEXTURE,
    MIXTURE
};

// Model position vectors
inline const std::vector<glm::vec3> modelPositions = common::utility::GenerateRandomPositions(
        MAX_NUM_OBJECTS, glm::vec3(-15.0f, -12.0f, -15.0f), glm::vec3(15.0f, 12.0f, -2.0f), 1.5f);

} // namespace examples::fundamentals::queries_and_performance::specialization_constants
