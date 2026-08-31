/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    31.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::advanced_compute_shaders::ray_tracing_compute
{
namespace constants
{
    // Shaders
    inline constexpr auto kPresentVertexShaderFile = "fullscreen_triangle.vert.spv";
    inline constexpr auto kPresentFragmentShaderFile = "present.frag.spv";
    inline constexpr auto kRayTraceComputeShaderFile = "raytrace_bvh.comp.spv";
    inline constexpr auto kPresentVertexShaderKey = "vertPresent";
    inline constexpr auto kPresentFragmentShaderKey = "fragPresent";
    inline constexpr auto kRayTraceComputeShaderKey = "compRayTrace";

    // Buffers
    inline constexpr auto kBvhNodeBuffer = "bvhNodeBuffer";
    inline constexpr auto kTriangleBuffer = "triangleBuffer";
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kRadianceImage = "radianceImage";
    inline constexpr auto kRadianceImageView = "radianceImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kSkyboxSampler = "skyboxSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kRayTraceDescSet = "rayTraceDescSet";
    inline constexpr auto kRayTraceDescSetLayout = "rayTraceDescSetLayout";
    inline constexpr auto kPresentDescSet = "presentDescSet";
    inline constexpr auto kPresentDescSetLayout = "presentDescSetLayout";

    // Textures
    inline constexpr auto kCubemapRightTexturePath = "Textures/calm_sea_cubemap/calm_sea_right.jpg";
    inline constexpr auto kCubemapLeftTexturePath = "Textures/calm_sea_cubemap/calm_sea_left.jpg";
    inline constexpr auto kCubemapTopTexturePath = "Textures/calm_sea_cubemap/calm_sea_up.jpg";
    inline constexpr auto kCubemapBottomTexturePath = "Textures/calm_sea_cubemap/calm_sea_down.jpg";
    inline constexpr auto kCubemapBackTexturePath = "Textures/calm_sea_cubemap/calm_sea_back.jpg";
    inline constexpr auto kCubemapFrontTexturePath = "Textures/calm_sea_cubemap/calm_sea_front.jpg";
    inline constexpr auto kCubemapTexture = "cubemapTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::ray_tracing_compute
