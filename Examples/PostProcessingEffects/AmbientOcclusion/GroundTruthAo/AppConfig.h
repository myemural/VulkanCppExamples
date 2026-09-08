/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    08.09.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::post_processing_effects::ambient_occlusion::ground_truth_ao
{
namespace constants
{
    // Shaders
    inline constexpr auto kGeometryVertexShaderFile = "geometry_pass.vert.spv";
    inline constexpr auto kGeometryFragmentShaderFile = "geometry_pass.frag.spv";
    inline constexpr auto kFullScreenQuadVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kGtaoFragmentShaderFile = "gtao_pass.frag.spv";
    inline constexpr auto kGtaoBlurFragmentShaderFile = "gtao_blur_pass.frag.spv";
    inline constexpr auto kLightFragmentShaderFile = "light_pass.frag.spv";
    inline constexpr auto kGeometryVertexShaderKey = "vertGeometry";
    inline constexpr auto kGeometryFragmentShaderKey = "fragGeometry";
    inline constexpr auto kFullScreenQuadVertexShaderKey = "vertFullScreenQuad";
    inline constexpr auto kGtaoFragmentShaderKey = "fragGtao";
    inline constexpr auto kGtaoBlurFragmentShaderKey = "fragGtaoBlur";
    inline constexpr auto kLightFragmentShaderKey = "fragLight";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kPositionImage = "positionImage";
    inline constexpr auto kPositionImageView = "positionImageView";
    inline constexpr auto kAlbedoImage = "albedoImage";
    inline constexpr auto kAlbedoImageView = "albedoImageView";
    inline constexpr auto kNormalImage = "normalImage";
    inline constexpr auto kNormalImageView = "normalImageView";
    inline constexpr auto kGtaoImage = "gtaoImage";
    inline constexpr auto kGtaoImageView = "gtaoImageView";
    inline constexpr auto kGtaoBlurImage = "gtaoBlurImage";
    inline constexpr auto kGtaoBlurImageView = "gtaoBlurImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kGBufferSampler = "gBufferSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kGtaoDescSet = "gtaoDescSet";
    inline constexpr auto kGtaoDescSetLayout = "gtaoDescSetLayout";
    inline constexpr auto kGtaoBlurDescSet = "gtaoBlurDescSet";
    inline constexpr auto kGtaoBlurDescSetLayout = "gtaoBlurDescSetLayout";
    inline constexpr auto kLightDescSet = "lightDescSet";
    inline constexpr auto kLightDescSetLayout = "lightDescSetLayout";

    // Textures
    inline constexpr auto kWallStoneTexturePath = "Textures/Wall_Stone_034_basecolor.png";
    inline constexpr auto kWallStoneTexture = "wallStoneTexture";
    inline constexpr auto kWallStoneNormalTexturePath = "Textures/Wall_Stone_034_normal.png";
    inline constexpr auto kWallStoneNormalTexture = "wallStoneNormalTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kRoomObject = "room";
    inline constexpr auto kPropObject = "prop";

    // Models
    inline constexpr auto kSuzanneModelPath = "Models/Suzanne.glb";
    inline constexpr auto kSuzanneModelName = "suzanne";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::post_processing_effects::ambient_occlusion::ground_truth_ao
