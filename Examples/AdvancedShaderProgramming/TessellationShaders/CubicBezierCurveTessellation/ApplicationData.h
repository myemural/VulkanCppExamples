/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    24.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::advanced_shader_programming::tessellation_shaders::cubic_bezier_curve_tessellation
{

// Constants
inline constexpr auto kPatchCount = 4;

struct PushConstants
{
    glm::vec4 shapeColor; // xyz = Color for control point or Bézier curve
    float controlPointSize = 1.0f;
    float tessLevel;
};

} // namespace examples::advanced_shader_programming::tessellation_shaders::cubic_bezier_curve_tessellation
