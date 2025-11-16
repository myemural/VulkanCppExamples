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

namespace examples::fundamentals::descriptor_sets::changing_color_with_ub
{

// Vertex Data for Triangle (Clockwise)
// clang-format off
inline const std::vector<common::utility::VertexPos2> vertices{
    {glm::vec2{0.0, -0.5}}, {glm::vec2{0.5, 0.5}}, {glm::vec2{-0.5, 0.5}}
};
// clang-format on
} // namespace examples::fundamentals::descriptor_sets::changing_color_with_ub
