/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    01.03.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::transparency_techniques::alpha_to_coverage_transparency
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "blinn_phong_directional.vert.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "blinn_phong_directional.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kSceneObjectsFragmentShaderKey = "fragScene";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kResolveImage = "resolveImage";
    inline constexpr auto kResolveImageView = "resolveImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";

    // Textures
    inline constexpr auto kWallStoneTexturePath = "Textures/Wall_Stone_034_basecolor.png";
    inline constexpr auto kWallStoneTexture = "wallStoneTexture";
    inline constexpr auto kWallStoneNormalTexturePath = "Textures/Wall_Stone_034_normal.png";
    inline constexpr auto kWallStoneNormalTexture = "wallStoneNormalTexture";
    inline constexpr auto kMetalChainTexturePath = "Textures/Metal_Chainlink_004_basecolor.png";
    inline constexpr auto kMetalChainTexture = "metalChainTexture";
    inline constexpr auto kMetalChainNormalTexturePath = "Textures/Metal_Chainlink_004_normal.png";
    inline constexpr auto kMetalChainNormalTexture = "metalChainNormalTexture";
    inline constexpr auto kMetalChainOpacityTexturePath = "Textures/Metal_Chainlink_004_opacity.png";
    inline constexpr auto kMetalChainOpacityTexture = "metalChainOpacityTexture";
    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kWallObject1 = "wallObject1";
    inline constexpr auto kWallObject2 = "wallObject2";
    inline constexpr auto kWallObject3 = "wallObject3";
    inline constexpr auto kWallObject4 = "wallObject4";
    inline constexpr auto kWallObject5 = "wallObject5";
    inline constexpr auto kWallObject6 = "wallObject6";
    inline constexpr auto kBottomPlane = "bottomPlane";
    inline constexpr auto kRedPlane = "redPlane";
    inline constexpr auto kBluePlane = "bluePlane";
    inline constexpr auto kGreenPlane = "greenPlane";
    inline constexpr auto kCubeObject = "cube";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::real_time_lighting::transparency_techniques::alpha_to_coverage_transparency
