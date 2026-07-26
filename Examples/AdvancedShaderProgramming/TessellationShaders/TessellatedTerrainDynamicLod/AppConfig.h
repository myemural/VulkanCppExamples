/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    26.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::tessellation_shaders::tessellated_terrain_dynamic_lod
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "blinn_phong_directional.vert.spv";
    inline constexpr auto kMainTessControlShaderFile = "terrain_tessellation.tesc.spv";
    inline constexpr auto kMainTessEvaluationShaderFile = "terrain_tessellation.tese.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "blinn_phong_directional.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainTessControlShaderKey = "tescMain";
    inline constexpr auto kMainTessEvaluationShaderKey = "teseMain";
    inline constexpr auto kSceneObjectsFragmentShaderKey = "fragScene";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kTerrainSampler = "terrainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";

    // Textures
    inline constexpr auto kGroundTexturePath = "Textures/Ground_Dirt_007_basecolor.jpg";
    inline constexpr auto kGroundTexture = "groundTexture";
    inline constexpr auto kGroundNormalTexturePath = "Textures/Ground_Dirt_007_normal.jpg";
    inline constexpr auto kGroundNormalTexture = "groundNormalTexture";
    inline constexpr auto kHeightmapTexturePath = "Textures/heightmap.png";
    inline constexpr auto kHeightmapTexture = "heightmapTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kTerrain = "terrain";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto PolygonMode = "AppSettings.PolygonMode";
    constexpr auto MinTessDistance = "AppSettings.MinTessDistance";
    constexpr auto MaxTessDistance = "AppSettings.MaxTessDistance";
    constexpr auto MinTessLevel = "AppSettings.MinTessLevel";
    constexpr auto MaxTessLevel = "AppSettings.MaxTessLevel";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::tessellation_shaders::tessellated_terrain_dynamic_lod
