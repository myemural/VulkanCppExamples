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

#include "glm/glm.hpp"

#include "PrimitiveUtils.h"
#include "Vertex.h"

namespace examples::fundamentals::compute_shaders::basic_particles
{

#define LOCAL_SIZE_X 256
#define NUM_CUBES 10

// Vertex Attribute Layout
struct VertexPos3Uv2
{
    common::utility::Attribute<common::utility::Vec3, 0> Position; // layout(location=0) in vec3 position;
    common::utility::Attribute<common::utility::Vec2, 1> Uv;       // layout(location=1) in vec2 texCoord;
};

// Vertex Data for Cubes
const std::vector cubeVertices = common::utility::CreateCubeVertices<VertexPos3Uv2>(1.0f);

// Index Data for Quad
const std::vector<uint16_t> cubeIndices = common::utility::CreateCubeIndices();

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
inline constexpr glm::vec3 cubePositions[NUM_CUBES] = {glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.4f, 0.8f, -0.6f),
                                                       glm::vec3(-2.2f, 1.6f, 0.9f),   glm::vec3(1.3f, -2.5f, 1.4f),
                                                       glm::vec3(-3.0f, -1.2f, -1.6f), glm::vec3(3.1f, 2.1f, 0.5f),
                                                       glm::vec3(-3.4f, 2.2f, -0.9f),  glm::vec3(0.9f, -3.4f, -1.3f),
                                                       glm::vec3(-1.5f, -3.1f, 2.0f),  glm::vec3(3.3f, -0.6f, 2.4f)};

} // namespace examples::fundamentals::compute_shaders::basic_particles
