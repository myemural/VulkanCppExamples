/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    21.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include "Vertex.h"

namespace examples::fundamentals::basics::drawing_quad
{

// Vertex Data for Quad
// clang-format off
inline const std::vector<common::utility::VertexPos2Color3> vertices {
    {glm::vec2{-0.5, -0.5}, glm::vec3{1.0f, 0.0f, 0.0f}},
    {glm::vec2{0.5, -0.5}, glm::vec3{0.0f, 1.0f, 0.0f}},
    {glm::vec2{0.5, 0.5}, glm::vec3{0.0f, 0.0f, 1.0f}},
    {glm::vec2{-0.5, 0.5}, glm::vec3{1.0f, 0.0f, 1.0f}}
};
// clang-format on

// Index Data for Quad
// clang-format off
inline const std::vector<uint16_t> indices{
    0, 1, 2, // First triangle of quad
    2, 3, 0  // Second triangle of quad
};
// clang-format on
} // namespace examples::fundamentals::basics::drawing_quad
