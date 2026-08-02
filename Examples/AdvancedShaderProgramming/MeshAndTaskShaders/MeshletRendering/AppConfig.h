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

namespace examples::advanced_shader_programming::mesh_and_task_shaders::meshlet_rendering
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainMeshShaderFile = "meshlet_rendering.mesh.spv";
    inline constexpr auto kMainFragmentShaderFile = "meshlet_rendering.frag.spv";
    inline constexpr auto kMainMeshShaderKey = "meshMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";

    // Buffers
    constexpr auto kMeshletVertexBuffer = "meshletVertexBuffer";
    constexpr auto kMeshletVertexIndicesBuffer = "meshletVertexIndicesBuffer";
    constexpr auto kMeshletTriangleIndicesBuffer = "meshletTriangleIndicesBuffer";
    constexpr auto kMeshletDescriptorBuffer = "meshletDescriptorBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";

    // Models
    inline constexpr auto kSuzanneDetailedModelPath = "Models/SuzanneDetailed.glb";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::meshlet_rendering
