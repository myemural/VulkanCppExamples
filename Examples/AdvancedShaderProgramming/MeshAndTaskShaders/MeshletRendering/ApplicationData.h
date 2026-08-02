/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    02.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/mat4x4.hpp>

namespace examples::advanced_shader_programming::mesh_and_task_shaders::meshlet_rendering
{

// Constants
inline constexpr auto kObjectCount = 30;

struct MvpData
{
    glm::mat4 mvpMatrix;
};

} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::meshlet_rendering
