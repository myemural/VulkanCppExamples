/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    05.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/mat4x4.hpp>

namespace examples::advanced_shader_programming::mesh_and_task_shaders::frustum_culling_task_shader
{

// Constants
inline constexpr std::uint32_t kTaskGroupSize = 32U;

struct CullingPushConstants
{
    glm::mat4 modelMatrix;
    glm::mat4 projViewMatrix;
    std::uint32_t meshletCount;
};

struct LinePushConstants
{
    glm::mat4 viewProjMat;
};

} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::frustum_culling_task_shader
