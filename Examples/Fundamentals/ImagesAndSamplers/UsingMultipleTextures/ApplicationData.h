/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    01.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include "Vertex.h"

namespace examples::fundamentals::images_and_samplers::using_multiple_textures
{
#define NUM_QUADS 3

// Vertex Data for Quad
// clang-format off
inline const std::vector<common::utility::VertexPos2Uv2> vertices {
    {glm::vec2{-0.4, -0.4}, glm::vec2{1.0f, 0.0f}},
    {glm::vec2{0.4, -0.4}, glm::vec2{0.0f, 0.0f}},
    {glm::vec2{0.4, 0.4}, glm::vec2{0.0f, 1.0f}},
    {glm::vec2{-0.4, 0.4}, glm::vec2{1.0f, 1.0f}}
};
// clang-format on

// Index Data for Quad
// clang-format off
inline const std::vector<std::uint16_t> indices {
    0, 1, 2, // First triangle
    2, 3, 0  // Second triangle
};
// clang-format on

// Push Constant Data
struct alignas(8) PushConstantData
{
    glm::vec2 Offset;
    int SamplerIndex;
};

// Quad indices
enum QuadIndex
{
    TOP_LEFT_QUAD_INDEX = 0,
    TOP_RIGHT_QUAD_INDEX,
    BOTTOM_CENTER_QUAD_INDEX,
    QUAD_COUNT
};
} // namespace examples::fundamentals::images_and_samplers::using_multiple_textures
