/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    11.02.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::lighting_architectures::tiled_forward_shading
{
namespace constants
{
    // Shaders
    inline constexpr auto kDepthPrePassVertexShaderFile = "depth_pre_pass.vert.spv";
    inline constexpr auto kTiledForwardVertexShaderFile = "tiled_forward_pass.vert.spv";
    inline constexpr auto kTiledForwardFragmentShaderFile = "tiled_forward_pass.frag.spv";
    inline constexpr auto kTileLightCullComputeShaderFile = "tile_light_cull.comp.spv";
    inline constexpr auto kDepthPrePassVertexShaderKey = "vertDepthPrePass";
    inline constexpr auto kTiledForwardVertexShaderKey = "vertTiledForward";
    inline constexpr auto kTiledForwardFragmentShaderKey = "fragTiledForward";
    inline constexpr auto kTileLightCullComputeShaderKey = "compTileLightCull";

    // Buffers
    inline constexpr auto kPointLightStorageBuffer = "pointLightStorageBuffer";
    inline constexpr auto kTileLightListStorageBuffer = "tileLightListStorageBuffer";

    // Images and Image Views
    inline constexpr auto kDepthPrePassImage = "depthPrePassImage";
    inline constexpr auto kDepthPrePassImageView = "depthPrePassImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kDepthPrePassDescSet = "depthPrePassDescSet";
    inline constexpr auto kDepthPrePassDescSetLayout = "depthPrePassDescSetLayout";

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
} // namespace examples::real_time_lighting::lighting_architectures::tiled_forward_shading
