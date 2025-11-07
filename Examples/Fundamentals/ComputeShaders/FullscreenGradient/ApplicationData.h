/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    7.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include "Vertex.h"

namespace examples::fundamentals::compute_shaders::fullscreen_gradient
{
// Vertex Attribute Layout
struct VertexPos3Uv2
{
    common::utility::Attribute<common::utility::Vec3, 0> Position; // layout(location=0) in vec3 position;
    common::utility::Attribute<common::utility::Vec2, 1> Uv;       // layout(location=1) in vec2 texCoord;
};

// Vertex Data for Quad (XY)
const std::vector quadVertices{
    VertexPos3Uv2{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // 0
    VertexPos3Uv2{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},  // 1
    VertexPos3Uv2{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},   // 2
    VertexPos3Uv2{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},  // 3
};

// Index Data for Quad
const std::vector<uint16_t> quadIndices{
    0, 1, 2, // First triangle
    2, 3, 0  // Second triangle
};

} // namespace examples::fundamentals::compute_shaders::fullscreen_gradient
