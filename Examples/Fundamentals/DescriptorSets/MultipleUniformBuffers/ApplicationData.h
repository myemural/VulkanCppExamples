/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    23.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include "Vertex.h"

namespace examples::fundamentals::descriptor_sets::multiple_uniform_buffers
{
// Vertex Attribute Layout
struct VertexPos2
{
    common::utility::Attribute<common::utility::Vec2, 0> Position; // layout(location=0) in vec2 position;
};

// Vertex Data
const std::vector vertices{
    // Top-left triangle (Clockwise)
    VertexPos2{{-0.8, -0.8}},
    VertexPos2{{-0.2, -0.8}},
    VertexPos2{{-0.5, -0.2}},

    // Top-right triangle (Clockwise)
    VertexPos2{{0.2, -0.8}},
    VertexPos2{{0.8, -0.8}},
    VertexPos2{{0.5, -0.2}},

    // Bottom-left triangle (Clockwise)
    VertexPos2{{-0.5, 0.2}},
    VertexPos2{{-0.2, 0.8}},
    VertexPos2{{-0.8, 0.8}},

    // Bottom-right triangle (Clockwise)
    VertexPos2{{0.5, 0.2}},
    VertexPos2{{0.8, 0.8}},
    VertexPos2{{0.2, 0.8}}
};
} // namespace examples::fundamentals::descriptor_sets::multiple_uniform_buffers
