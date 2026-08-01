/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
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
namespace constants
{
    // Shaders
    inline constexpr auto kMainMeshShaderFile = "triangles_mesh.mesh.spv";
    inline constexpr auto kMainFragmentShaderFile = "triangles_mesh.frag.spv";
    inline constexpr auto kMainMeshShaderKey = "meshMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::triangles_with_mesh_shader
