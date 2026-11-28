/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    28.11.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::ray_traced_triangle
{
namespace constants
{
    // Shaders
    inline constexpr auto kRayGenShaderFile = "primary_ray.rgen.spv";
    inline constexpr auto kMissShaderFile = "background.rmiss.spv";
    inline constexpr auto kClosestHitShaderFile = "barycentric_color.rchit.spv";
    inline constexpr auto kPresentVertexShaderFile = "fullscreen_triangle.vert.spv";
    inline constexpr auto kPresentFragmentShaderFile = "present.frag.spv";
    inline constexpr auto kRayGenShaderKey = "rgenPrimaryRay";
    inline constexpr auto kMissShaderKey = "rmissBackground";
    inline constexpr auto kClosestHitShaderKey = "rchitBarycentricColor";
    inline constexpr auto kPresentVertexShaderKey = "vertPresent";
    inline constexpr auto kPresentFragmentShaderKey = "fragPresent";

    // Buffers
    inline constexpr auto kVertexBuffer = "vertexBuffer";
    inline constexpr auto kIndexBuffer = "indexBuffer";
    inline constexpr auto kInstanceBuffer = "instanceBuffer";
    inline constexpr auto kSceneUniformBuffer = "sceneUniformBuffer";

    // Acceleration Structures
    inline constexpr auto kTriangleBlas = "triangleBlas";
    inline constexpr auto kSceneTlas = "sceneTlas";

    // Images and Image Views
    inline constexpr auto kRadianceImage = "radianceImage";
    inline constexpr auto kRadianceImageView = "radianceImageView";

    // Samplers
    inline constexpr auto kPresentSampler = "presentSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kRayTraceDescSet = "rayTraceDescSet";
    inline constexpr auto kRayTraceDescSetLayout = "rayTraceDescSetLayout";
    inline constexpr auto kPresentDescSet = "presentDescSet";
    inline constexpr auto kPresentDescSetLayout = "presentDescSetLayout";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
} // namespace AppSettings
} // namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::ray_traced_triangle
