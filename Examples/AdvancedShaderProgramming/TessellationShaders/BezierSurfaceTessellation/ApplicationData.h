/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    25.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::advanced_shader_programming::tessellation_shaders::bezier_surface_tessellation
{

// Constants
inline constexpr auto kPatchCount = 16;
inline constexpr auto kControlPointPickRadius = 0.20f;

struct SurfacePushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    float tessLevel;
};

struct ControlPointsPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 pointColor;
    float pointSize;
};

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
};

} // namespace examples::advanced_shader_programming::tessellation_shaders::bezier_surface_tessellation
