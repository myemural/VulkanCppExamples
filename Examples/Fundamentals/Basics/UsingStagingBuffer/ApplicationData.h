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

// Vertex Data for Octagon (using TRIANGLE_FAN)
// clang-format off
const std::vector<common::utility::VertexPos2> vertices{
    {glm::vec2{0.8, 0.0}},
    {glm::vec2{0.6, 0.6}},
    {glm::vec2{0.0, 0.80}},
    {glm::vec2{-0.6, 0.6}},
    {glm::vec2{-0.80, 0.0}},
    {glm::vec2{-0.6, -0.6}},
    {glm::vec2{0.0, -0.80}},
    {glm::vec2{0.6, -0.6}}
};
// clang-format on
} // namespace examples::fundamentals::basics::using_staging_buffer
