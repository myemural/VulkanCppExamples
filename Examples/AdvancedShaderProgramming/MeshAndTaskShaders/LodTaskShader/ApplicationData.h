/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    06.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/mat4x4.hpp>

namespace examples::advanced_shader_programming::mesh_and_task_shaders::lod_task_shader
{

// Constants
inline constexpr auto kObjectCount = 30;
inline constexpr std::uint32_t kTaskGroupSize = 32U;
inline const std::vector kLodTargetRatios = {1.0f, 0.5f, 0.22f, 0.08f};
inline constexpr auto kLodDistances = glm::vec4(10.0f, 20.0f, 30.0f, 40.0f);

struct LodPushConstants
{
    glm::mat4 modelMatrix;
    glm::mat4 projViewMatrix;
    glm::vec4 cameraPos;    // xyz = Camera Position
    glm::vec4 lodDistances; // x = LOD0, y = LOD1, z = LOD2, w = LOD3
};

} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::lod_task_shader
