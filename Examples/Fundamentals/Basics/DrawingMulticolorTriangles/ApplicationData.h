/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    20.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Vertex.h"

namespace examples::fundamentals::basics::drawing_multicolor_triangles
{

// Vertex Data
// clang-format off
inline const std::vector<common::utility::VertexPos2Color3> vertices {
    // Top-left triangle (Clockwise)
    {glm::vec2{-0.8, -0.8}, glm::vec3{1.0f, 0.0f, 0.0f}},
    {glm::vec2{-0.2, -0.8}, glm::vec3{1.0f, 0.0f, 0.0f}},
    {glm::vec2{-0.5, -0.2}, glm::vec3{1.0f, 1.0f, 1.0f}},

    // Top-right triangle (Clockwise)
    {glm::vec2{0.2, -0.8}, glm::vec3{1.0f, 0.0f, 1.0f}},
    {glm::vec2{0.8, -0.8}, glm::vec3{1.0f, 0.0f, 1.0f}},
    {glm::vec2{0.5, -0.2}, glm::vec3{1.0f, 1.0f, 1.0f}},

    // Bottom-left triangle (Clockwise)
    {glm::vec2{-0.5, 0.2}, glm::vec3{1.0f, 1.0f, 1.0f}},
    {glm::vec2{-0.2, 0.8}, glm::vec3{0.0f, 0.0f, 1.0f}},
    {glm::vec2{-0.8, 0.8}, glm::vec3{0.0f, 0.0f, 1.0f}},

    // Bottom-right triangle (Clockwise)
    {glm::vec2{0.5, 0.2}, glm::vec3{1.0f, 1.0f, 1.0f}},
    {glm::vec2{0.8, 0.8}, glm::vec3{1.0f, 1.0f, 0.0f}},
    {glm::vec2{0.2, 0.8}, glm::vec3{1.0f, 1.0f, 0.0f}}
};
// clang-format on
} // namespace examples::fundamentals::basics::drawing_multicolor_triangles
