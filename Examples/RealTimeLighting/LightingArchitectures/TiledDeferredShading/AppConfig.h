/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    16.02.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::lighting_architectures::tiled_deferred_shading
{
namespace constants
{
    // Shaders
    inline constexpr auto kGeometryVertexShaderFile = "geometry_pass.vert.spv";
    inline constexpr auto kGeometryFragmentShaderFile = "geometry_pass.frag.spv";
    inline constexpr auto kTileLightCullComputeShaderFile = "tile_light_cull.comp.spv";
    inline constexpr auto kLightVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kLightFragmentShaderFile = "light_pass.frag.spv";
    inline constexpr auto kGeometryVertexShaderKey = "vertGeometry";
    inline constexpr auto kGeometryFragmentShaderKey = "fragGeometry";
    inline constexpr auto kTileLightCullComputeShaderKey = "compTileLightCull";
    inline constexpr auto kLightVertexShaderKey = "vertLight";
    inline constexpr auto kLightFragmentShaderKey = "fragLight";

    // Buffers
    inline constexpr auto kPointLightStorageBuffer = "pointLightStorageBuffer";
    inline constexpr auto kTileLightListStorageBuffer = "tileLightListStorageBuffer";

    // Images and Image Views
    inline constexpr auto kPositionImage = "positionImage";
    inline constexpr auto kPositionImageView = "positionImageView";
    inline constexpr auto kAlbedoImage = "albedoImage";
    inline constexpr auto kAlbedoImageView = "albedoImageView";
    inline constexpr auto kNormalImage = "normalImage";
    inline constexpr auto kNormalImageView = "normalImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kGeometryPassDescSet = "geometryPassDescSet";
    inline constexpr auto kGeometryPassDescSetLayout = "geometryPassDescSetLayout";
    inline constexpr auto kLightCullDescSet = "lightCullDescSet";
    inline constexpr auto kLightCullDescSetLayout = "lightCullDescSetLayout";
    inline constexpr auto kLightPassDescSet = "lightPassDescSet";
    inline constexpr auto kLightPassDescSetLayout = "lightPassDescSetLayout";

    // Textures
    inline constexpr auto kWallStoneTexturePath = "Textures/Wall_Stone_034_basecolor.png";
    inline constexpr auto kWallStoneTexture = "wallStoneTexture";
    inline constexpr auto kWallStoneNormalTexturePath = "Textures/Wall_Stone_034_normal.png";
    inline constexpr auto kWallStoneNormalTexture = "wallStoneNormalTexture";

    // Materials
    inline constexpr auto kDefaultMaterial = "defaultMaterial";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kCubeObject = "cube";
    inline constexpr auto kSphereObject = "sphere";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto LightRadius = "AppSettings.LightRadius";
} // namespace AppSettings
} // namespace examples::real_time_lighting::lighting_architectures::tiled_deferred_shading
