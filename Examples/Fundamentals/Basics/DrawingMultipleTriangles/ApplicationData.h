/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include "Vertex.h"

namespace examples::fundamentals::basics::drawing_multiple_triangles
{

// Vertex Data
// clang-format off
inline const std::vector<common::utility::VertexPos2> vertices {
    // Top-left triangle (Clockwise)
    {glm::vec2{-0.8, -0.8}}, {glm::vec2{-0.2, -0.8}}, {glm::vec2{-0.5, -0.2}},

    // Top-right triangle (Clockwise)
    {glm::vec2{0.2, -0.8}}, {glm::vec2{0.8, -0.8}}, {glm::vec2{0.5, -0.2}},

    // Bottom-left triangle (Clockwise)
    {glm::vec2{-0.5, 0.2}}, {glm::vec2{-0.2, 0.8}}, {glm::vec2{-0.8, 0.8}},

    // Bottom-right triangle (Clockwise)
    {glm::vec2{0.5, 0.2}}, {glm::vec2{0.8, 0.8}}, {glm::vec2{0.2, 0.8}}
};
// clang-format on
} // namespace examples::fundamentals::basics::drawing_multiple_triangles
