/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    10.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include "PrimitiveUtils.h"
#include "Vertex.h"

namespace examples::fundamentals::compute_shaders::mandelbrot_fractal
{

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

// Vertex Data for Quad (XY)
inline const std::vector quadVertices = common::utility::CreateQuadVerticesXY<common::utility::VertexPos3Uv2>(1.0f);

// Index Data for Quad (XY)
inline const std::vector<std::uint16_t> quadIndices = common::utility::CreateQuadIndices();

struct MandelbrotPushConstants
{
    glm::vec2 center;
    float loopTime;
    float zoomSpeed;
    float time;
};

} // namespace examples::fundamentals::compute_shaders::mandelbrot_fractal
