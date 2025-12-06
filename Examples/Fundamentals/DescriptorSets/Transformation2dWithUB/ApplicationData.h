/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    24.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include <glm/mat4x4.hpp>

#include "Vertex.h"

namespace examples::fundamentals::descriptor_sets::transformation2d_with_ub
{

// Vertex Data for Quad
// clang-format off
inline const std::vector<common::utility::VertexPos2> vertices {
    {glm::vec2{-0.25, -0.25}}, // 0
    {glm::vec2{0.25, -0.25}},  // 1
    {glm::vec2{0.25, 0.25}},   // 2
    {glm::vec2{-0.25, 0.25}}   // 3
};
// clang-format on

// Index Data for Quad
// clang-format off
inline const std::vector<std::uint16_t> indices {
    0, 1, 2, // First triangle
    2, 3, 0  // Second triangle
};
// clang-format on

// Model Matrix (for Uniform Buffer)
struct UniformBufferObject
{
    glm::mat4 model;
};

} // namespace examples::fundamentals::descriptor_sets::transformation2d_with_ub
