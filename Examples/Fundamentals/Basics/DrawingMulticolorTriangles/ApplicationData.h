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

#include "Vertex.h"

namespace examples::fundamentals::basics::drawing_multicolor_triangles
{

// Vertex Attribute Layout
struct VertexPos2Color3
{
    common::utility::Attribute<common::utility::Vec2, 0> Position; // layout(location=0) in vec2 position;
    common::utility::Attribute<common::utility::Color3, 1> Color;  // layout(location=1) in vec3 color;
};

// Vertex Data
const std::vector vertices{
    // Top-left triangle (Clockwise)
    VertexPos2Color3{{-0.8, -0.8}, {1.0f, 0.0f, 0.0f}},
    VertexPos2Color3{{-0.2, -0.8}, {1.0f, 0.0f, 0.0f}},
    VertexPos2Color3{{-0.5, -0.2}, {1.0f, 1.0f, 1.0f}},

    // Top-right triangle (Clockwise)
    VertexPos2Color3{{0.2, -0.8}, {1.0f, 0.0f, 1.0f}},
    VertexPos2Color3{{0.8, -0.8}, {1.0f, 0.0f, 1.0f}},
    VertexPos2Color3{{0.5, -0.2}, {1.0f, 1.0f, 1.0f}},

    // Bottom-left triangle (Clockwise)
    VertexPos2Color3{{-0.5, 0.2}, {1.0f, 1.0f, 1.0f}},
    VertexPos2Color3{{-0.2, 0.8}, {0.0f, 0.0f, 1.0f}},
    VertexPos2Color3{{-0.8, 0.8}, {0.0f, 0.0f, 1.0f}},

    // Bottom-right triangle (Clockwise)
    VertexPos2Color3{{0.5, 0.2}, {1.0f, 1.0f, 1.0f}},
    VertexPos2Color3{{0.8, 0.8}, {1.0f, 1.0f, 0.0f}},
    VertexPos2Color3{{0.2, 0.8}, {1.0f, 1.0f, 0.0f}}
};

} // namespace examples::fundamentals::basics::drawing_multicolor_triangles