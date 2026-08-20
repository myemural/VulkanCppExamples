/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    19.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::advanced_compute_shaders::occlusion_culling_compute
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "blinn_phong_directional.vert.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "blinn_phong_directional.frag.spv";
    inline constexpr auto kHizCopyComputeShaderFile = "hiz_depth_copy.comp.spv";
    inline constexpr auto kHizReduceComputeShaderFile = "hiz_reduce.comp.spv";
    inline constexpr auto kOcclusionComputeShaderFile = "occlusion_culling.comp.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kSceneObjectsFragmentShaderKey = "fragScene";
    inline constexpr auto kHizCopyComputeShaderKey = "compHizCopy";
    inline constexpr auto kHizReduceComputeShaderKey = "compHizReduce";
    inline constexpr auto kOcclusionComputeShaderKey = "compOcclusion";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";
    inline constexpr auto kObjectBoundsBuffer = "objectBoundsBuffer";
    inline constexpr auto kCullingResultsBuffer = "cullingResultsBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";
    inline constexpr auto kHizImage = "hizImage";
    inline constexpr auto kHizImageViewAllMips = "hizImageViewAllMips";
    inline constexpr auto kHizImageViewMipPrefix = "hizImageViewMip";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kPointSampler = "pointSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kHizCopyDescSet = "hizCopyDescSet";
    inline constexpr auto kHizCopyDescSetLayout = "hizCopyDescSetLayout";
    inline constexpr auto kHizReduceDescSetPrefix = "hizReduceDescSet";
    inline constexpr auto kHizReduceDescSetLayout = "hizReduceDescSetLayout";
    inline constexpr auto kOcclusionDescSet = "occlusionDescSet";
    inline constexpr auto kOcclusionDescSetLayout = "occlusionDescSetLayout";

    // Textures
    inline constexpr auto kWallStoneTexturePath = "Textures/Wall_Stone_034_basecolor.png";
    inline constexpr auto kWallStoneTexture = "wallStoneTexture";
    inline constexpr auto kWallStoneNormalTexturePath = "Textures/Wall_Stone_034_normal.png";
    inline constexpr auto kWallStoneNormalTexture = "wallStoneNormalTexture";

    inline constexpr auto kTilesTexturePath = "Textures/Tiles_069_basecolor.png";
    inline constexpr auto kTilesTexture = "tilesTexture";
    inline constexpr auto kTilesNormalTexturePath = "Textures/Tiles_069_normal.png";
    inline constexpr auto kTilesNormalTexture = "tilesNormalTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kCubeObject = "cube";
    inline constexpr auto kPlaneOccluderObject = "planeOccluder";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::occlusion_culling_compute
