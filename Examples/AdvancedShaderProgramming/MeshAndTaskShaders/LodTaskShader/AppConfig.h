/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    06.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::mesh_and_task_shaders::lod_task_shader
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainTaskShaderFile = "lod_rendering_cull.task.spv";
    inline constexpr auto kMainMeshShaderFile = "lod_rendering.mesh.spv";
    inline constexpr auto kMainFragmentShaderFile = "lod_rendering.frag.spv";
    inline constexpr auto kMainTaskShaderKey = "taskMain";
    inline constexpr auto kMainMeshShaderKey = "meshMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";

    // Buffers
    inline constexpr auto kMeshletVertexBuffer = "meshletVertexBuffer";
    inline constexpr auto kMeshletVertexIndicesBuffer = "meshletVertexIndicesBuffer";
    inline constexpr auto kMeshletTriangleIndicesBuffer = "meshletTriangleIndicesBuffer";
    inline constexpr auto kMeshletDescriptorBuffer = "meshletDescriptorBuffer";
    inline constexpr auto kLodInfoBuffer = "lodInfoBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kMainDescSet = "mainDescSet";

    // Models
    inline constexpr auto kSuzanneDetailedModelPath = "Models/SuzanneDetailed.glb";
} // namespace constants

namespace AppSettings
{
    inline constexpr auto ClearColor = "AppSettings.ClearColor";
    inline constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    inline constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto PolygonMode = "AppSettings.PolygonMode";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::lod_task_shader
