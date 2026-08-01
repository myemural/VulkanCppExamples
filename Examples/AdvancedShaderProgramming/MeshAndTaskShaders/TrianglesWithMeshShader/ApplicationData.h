/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    31.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::mesh_and_task_shaders::triangles_with_mesh_shader
{

struct MeshPushConstants
{
    std::uint32_t triangleCount;
};

} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::triangles_with_mesh_shader
