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

#include "Vertex.h"
#include "glm/glm.hpp"

namespace examples::fundamentals::descriptor_sets::array_of_ub
{
// Vertex Attribute Layout
struct VertexPos2
{
    common::utility::Attribute<common::utility::Vec2, 0> Position; // layout(location=0) in vec2 position;
};

// Vertex Data
const std::vector vertices{
    // Square
    VertexPos2{{-0.25, -0.25}}, // 0
    VertexPos2{{0.25, -0.25}}, // 1
    VertexPos2{{0.25, 0.25}}, // 2
    VertexPos2{{-0.25, 0.25}} // 3
};

// Index Data
const std::vector<std::uint16_t> indices{
    0, 1, 2, // First triangle
    2, 3, 0 // Second triangle
};

// Model Matrix (for Uniform Buffer)
struct UniformBufferObject
{
    glm::mat4 model;
};

inline UniformBufferObject modelUbObject[4] = {glm::mat4(1.0), glm::mat4(1.0), glm::mat4(1.0), glm::mat4(1.0)};
} // namespace examples::fundamentals::descriptor_sets::array_of_ub
