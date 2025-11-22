/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    22.11.2025
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

namespace examples::fundamentals::queries_and_performance::pipeline_statistics_query
{
#define NUM_CUBES 20
#define NUM_CUBES_PIPELINE_1 3
#define NUM_CUBES_PIPELINE_2 7
#define NUM_CUBES_PIPELINE_3 4
#define NUM_CUBES_PIPELINE_4 6

// Vertex Data for Cube
inline const std::vector cubeVertices = common::utility::CreateCubeVertices<common::utility::VertexPos3Uv2>(1.0f);

// Index Data for Cube
inline const std::vector<uint16_t> cubeIndices = common::utility::CreateCubeIndices();

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};

// Struct for keep pipeline statistics
struct PipelineStats
{
    std::uint64_t inputAssemblyVertices;
    std::uint64_t inputAssemblyPrimitives;
    std::uint64_t vertexShaderInvocations;
    std::uint64_t fragmentShaderInvocations;
};

// Model position vectors
// clang-format off
inline constexpr glm::vec3 modelPositions[NUM_CUBES] = {
    // Top-left cubes for pipeline 1 (3 cubes)
    { -2.0f,  2.0f, -2.0f },
    { -1.0f,  2.5f, -3.5f },
    { -1.5f,  1.5f, -5.0f },

    // Top-right cubes for pipeline 2 (7 cubes)
    {  2.0f,  2.0f, -2.5f },
    {  2.5f,  2.5f, -4.0f },
    {  3.0f,  1.5f, -5.5f },
    {  3.5f,  2.0f, -7.0f },
    {  2.0f,  2.2f, -8.0f },
    {  2.8f,  1.8f, -9.0f },
    {  3.2f,  2.3f, -10.0f },

    // Bottom-left cubes for pipeline 3 (4 cubes)
    { -2.0f, -2.0f, -2.5f },
    { -1.5f, -2.5f, -4.0f },
    { -2.5f, -1.5f, -5.5f },
    { -1.8f, -2.2f, -7.0f },

    // Bottom-right cubes for pipeline 4 (6 cubes)
    {  2.0f, -2.0f, -3.0f },
    {  2.5f, -2.5f, -4.5f },
    {  3.0f, -1.5f, -6.0f },
    {  3.5f, -2.0f, -7.5f },
    {  2.8f, -2.2f, -9.0f },
    {  3.2f, -1.8f, -10.5f }
};
// clang-format on

} // namespace examples::fundamentals::queries_and_performance::pipeline_statistics_query
