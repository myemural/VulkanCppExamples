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

namespace examples::fundamentals::basics::drawing_with_different_topology
{

// Vertex Data for Quads
// clang-format off
inline const std::vector<common::utility::VertexPos2Color3> vertices {
    // First rectangle (left)
    {glm::vec2{-0.8f, -0.5f}, glm::vec3{1.0f, 0.0f, 0.0f}},
    {glm::vec2{-0.4f, -0.5f}, glm::vec3{0.0f, 1.0f, 0.0f}},
    {glm::vec2{-0.8f, 0.5f}, glm::vec3{0.0f, 0.0f, 1.0f}},
    {glm::vec2{-0.4f, 0.5f}, glm::vec3{1.0f, 1.0f, 0.0f}},

    // Second rectangle (right)
    {glm::vec2{0.4f, -0.5f}, glm::vec3{1.0f, 0.0f, 1.0f}},
    {glm::vec2{0.8f, -0.5f}, glm::vec3{0.0f, 1.0f, 1.0f}},
    {glm::vec2{0.4f, 0.5f}, glm::vec3{1.0f, 1.0f, 1.0f}},
    {glm::vec2{0.8f, 0.5f}, glm::vec3{1.0f, 1.0f, 1.0f}}
};
// clang-format on

// Index Data for Quads (with restart index)
// clang-format off
inline const std::vector<std::uint16_t> indicesWithRestart {
    0,      1, 2, 3, // First strip
    0xFFFF,          // Primitive restart index
    4,      5, 6, 7  // Second strip
};
// clang-format on

// Index Data for Quads (without restart index)
// clang-format off
inline const std::vector<std::uint16_t> indicesWithoutRestart {
    0, 1, 2, 3,
    4, 5, 6, 7
};
// clang-format on
} // namespace examples::fundamentals::basics::drawing_with_different_topology
