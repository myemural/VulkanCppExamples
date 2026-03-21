/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    18.02.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::lighting_architectures::clustered_deferred_shading
{
namespace constants
{
    // Shaders
    inline constexpr auto kGeometryVertexShaderFile = "geometry_pass.vert.spv";
    inline constexpr auto kGeometryFragmentShaderFile = "geometry_pass.frag.spv";
    inline constexpr auto kClusterCountOffsetComputeShaderFile = "cluster_count_offset.comp.spv";
    inline constexpr auto kClusterLightIndicesComputeShaderFile = "cluster_light_indices.comp.spv";
    inline constexpr auto kLightVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kLightFragmentShaderFile = "light_pass.frag.spv";
    inline constexpr auto kGeometryVertexShaderKey = "vertGeometry";
    inline constexpr auto kGeometryFragmentShaderKey = "fragGeometry";
    inline constexpr auto kLightVertexShaderKey = "vertLight";
    inline constexpr auto kLightFragmentShaderKey = "fragLight";
    inline constexpr auto kClusterCountOffsetComputeShaderKey = "compClusterCountOffset";
    inline constexpr auto kClusterLightIndicesComputeShaderKey = "compClusterLightIndices";

    // Buffers
    inline constexpr auto kPointLightStorageBuffer = "pointLightStorageBuffer";
    inline constexpr auto kClusterHeaderStorageBuffer = "clusterHeaderStorageBuffer";
    inline constexpr auto kGlobalLightIndexStorageBuffer = "globalLightIndexStorageBuffer";
    inline constexpr auto kGlobalCounterStorageBuffer = "globalCounterStorageBuffer";

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
    inline constexpr auto kCountOffsetComputePassDescSet = "countOffsetComputePassDescSet";
    inline constexpr auto kLightIndicesComputePassDescSet = "lightIndicesComputePassDescSet";
    inline constexpr auto kComputePassDescSetLayout = "computePassDescSetLayout";
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
    inline constexpr auto kObject = "object";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto CameraNearPlane = "AppSettings.CameraNearPlane";
    constexpr auto CameraFarPlane = "AppSettings.CameraFarPlane";
    constexpr auto LightRadius = "AppSettings.LightRadius";
} // namespace AppSettings
} // namespace examples::real_time_lighting::lighting_architectures::clustered_deferred_shading
