/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    02.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::mesh_and_task_shaders::objects_mesh_shader_buffer
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainMeshShaderFile = "object_rendering.mesh.spv";
    inline constexpr auto kMainFragmentShaderFile = "blinn_phong.frag.spv";
    inline constexpr auto kMainMeshShaderKey = "meshMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";

    // Buffers
    constexpr auto kPositionBuffer = "positionBuffer";
    constexpr auto kNormalBuffer = "normalBuffer";
    constexpr auto kIndexBuffer = "indexBuffer";
    constexpr auto kSceneUniformBuffer = "sceneUniformBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::objects_mesh_shader_buffer
