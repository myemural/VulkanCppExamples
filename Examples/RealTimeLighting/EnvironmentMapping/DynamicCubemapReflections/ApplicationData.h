/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    25.01.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::real_time_lighting::environment_mapping::dynamic_cubemap_reflections
{

#define MOVING_OBJECT_COUNT 6U
#define CUBEMAP_RESOLUTION 256U

struct alignas(16) LightUbo
{
    glm::vec4 lightDirection; // xyz = Light Direction
    glm::vec4 lightColor;     // xyz = Light Color
};

constexpr glm::vec3 kReflectiveObjectPosition{0.0f, 0.0f, 0.0f};

} // namespace examples::real_time_lighting::environment_mapping::dynamic_cubemap_reflections
