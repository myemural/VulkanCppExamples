/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    13.11.2025
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

namespace examples::fundamentals::compute_shaders::basic_particles
{

#define LOCAL_SIZE_X 256
#define NUM_CUBES 10

// Vertex Data for Cubes
const std::vector cubeVertices = common::utility::CreateCubeVertices<common::utility::VertexPos3Uv2>(1.0f);

// Index Data for Quad
const std::vector<std::uint16_t> cubeIndices = common::utility::CreateCubeIndices();

struct MvpData
{
    glm::mat4 mvpMatrix;
};

struct alignas(16) ParticleData
{
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
};

struct ParticlePushConstant
{
    float deltaTime;
    int particleCount;
};

// Cube position vectors
inline const std::vector<glm::vec3> cubePositions = common::utility::GenerateRandomPositions(
        NUM_CUBES, glm::vec3(-4.0f, -4.0f, -4.0f), glm::vec3(4.0f, 4.0f, -1.0f), 2.0f);

} // namespace examples::fundamentals::compute_shaders::basic_particles
