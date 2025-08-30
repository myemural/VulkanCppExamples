/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    28.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include "Vertex.h"

namespace examples::fundamentals::images_and_samplers::textured_quad
{

// Vertex Attribute Layout
struct VertexPos2Uv2
{
    common::utility::Attribute<common::utility::Vec2, 0> Position; // layout(location=0) in vec2 position;
    common::utility::Attribute<common::utility::Vec2, 1> Uv;       // layout(location=1) in vec2 texCoord;
};

// Vertex Data
const std::vector vertices{
    // Quad
    VertexPos2Uv2{{-0.5, -0.5}, {1.0f, 0.0f}},
    VertexPos2Uv2{{0.5, -0.5}, {0.0f, 0.0f}},
    VertexPos2Uv2{{0.5, 0.5}, {0.0f, 1.0f}},
    VertexPos2Uv2{{-0.5, 0.5}, {1.0f, 1.0f}}
};

// Index Data
const std::vector<uint16_t> indices{
    0, 1, 2, // First triangle of quad
    2, 3, 0  // Second triangle of quad
};

} // namespace examples::fundamentals::images_and_samplers::textured_quad