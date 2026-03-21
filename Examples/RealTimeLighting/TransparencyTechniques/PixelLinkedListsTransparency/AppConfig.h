/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    20.03.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::transparency_techniques::pixel_linked_list_transparency
{
namespace constants
{
    // Shaders
    inline constexpr auto kGeometryVertexShaderFile = "geometry_pass.vert.spv";
    inline constexpr auto kGeometryFragmentShaderFile = "geometry_pass.frag.spv";
    inline constexpr auto kResolveVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kResolveFragmentShaderFile = "resolve_pass.frag.spv";
    inline constexpr auto kGeometryVertexShaderKey = "vertGeometry";
    inline constexpr auto kGeometryFragmentShaderKey = "fragGeometry";
    inline constexpr auto kResolveVertexShaderKey = "vertResolve";
    inline constexpr auto kResolveFragmentShaderKey = "fragResolve";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";
    inline constexpr auto kOitLinkedListBuffer = "oitLinkedListBuffer";
    inline constexpr auto kOitHeadPointerBuffer = "oitHeadPointerBuffer";
    inline constexpr auto kOitAtomicCounterBuffer = "oitAtomicCounterBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kGeometryDescSet = "geometryDescSet";
    inline constexpr auto kGeometryDescSetLayout = "geometryDescSetLayout";
    inline constexpr auto kResolveDescSet = "resolveDescSet";
    inline constexpr auto kResolveDescSetLayout = "resolveDescSetLayout";

    // Textures
    inline constexpr auto kWallStoneTexturePath = "Textures/Wall_Stone_034_basecolor.png";
    inline constexpr auto kWallStoneTexture = "wallStoneTexture";
    inline constexpr auto kWallStoneNormalTexturePath = "Textures/Wall_Stone_034_normal.png";
    inline constexpr auto kWallStoneNormalTexture = "wallStoneNormalTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kCubeObject = "cube";
    inline constexpr auto kSphereObject = "sphere";
    inline constexpr auto kPlaneObject = "plane";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto LightDirection = "AppSettings.LightDirection";
    constexpr auto LightColor = "AppSettings.LightColor";
    constexpr auto AmbientStrength = "AppSettings.AmbientStrength";
    constexpr auto SpecularStrength = "AppSettings.SpecularStrength";
    constexpr auto Shininess = "AppSettings.Shininess";
} // namespace AppSettings
} // namespace examples::real_time_lighting::transparency_techniques::pixel_linked_list_transparency
