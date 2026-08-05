/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    05.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::mesh_and_task_shaders::frustum_culling_task_shader
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainTaskShaderFile = "meshlet_rendering_cull.task.spv";
    inline constexpr auto kMainMeshShaderFile = "meshlet_rendering.mesh.spv";
    inline constexpr auto kMainFragmentShaderFile = "meshlet_rendering.frag.spv";
    inline constexpr auto kLineVertShaderFile = "frustum_line.vert.spv";
    inline constexpr auto kLineFragShaderFile = "frustum_line.frag.spv";
    inline constexpr auto kMainTaskShaderKey = "taskMain";
    inline constexpr auto kMainMeshShaderKey = "meshMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";
    inline constexpr auto kLineVertShaderKey = "vertLine";
    inline constexpr auto kLineFragShaderKey = "fragLine";

    // Buffers
    inline constexpr auto kMeshletVertexBuffer = "meshletVertexBuffer";
    inline constexpr auto kMeshletVertexIndicesBuffer = "meshletVertexIndicesBuffer";
    inline constexpr auto kMeshletTriangleIndicesBuffer = "meshletTriangleIndicesBuffer";
    inline constexpr auto kMeshletDescriptorBuffer = "meshletDescriptorBuffer";
    inline constexpr auto kMeshletBoundsBuffer = "meshletBoundsBuffer";
    inline constexpr auto kFrustumPlanesBuffer = "frustumPlanesBuffer";
    inline constexpr auto kFrustumCornersBuffer = "frustumCornersBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kLineDescSetLayout = "lineDescSetLayout";
    inline constexpr auto kLineDescSet = "lineDescSet";

    // Models
    inline constexpr auto kSuzanneDetailedModelPath = "Models/SuzanneDetailed.glb";
} // namespace constants

namespace AppSettings
{
    inline constexpr auto ClearColor = "AppSettings.ClearColor";
    inline constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    inline constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::frustum_culling_task_shader
