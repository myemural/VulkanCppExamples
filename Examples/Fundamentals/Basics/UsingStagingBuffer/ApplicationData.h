/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    22.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include "Vertex.h"

namespace examples::fundamentals::basics::using_staging_buffer
{

// Vertex Attribute Layout
struct VertexPos2
{
    common::utility::Attribute<common::utility::Vec2, 0> Position; // layout(location=0) in vec2 position;
};

// Vertex Data
const std::vector vertices{
    // Octagon (using TRIANGLE_FAN)
    VertexPos2{{0.8,  0.0}},
    VertexPos2{{0.6,  0.6}},
    VertexPos2{{0.0,  0.80}},
    VertexPos2{{-0.6,  0.6}},
    VertexPos2{{-0.80,  0.0}},
    VertexPos2{{-0.6, -0.6}},
    VertexPos2{{0.0, -0.80}},
    VertexPos2{{ 0.6, -0.6}}
};

} // namespace examples::fundamentals::basics::using_staging_buffer