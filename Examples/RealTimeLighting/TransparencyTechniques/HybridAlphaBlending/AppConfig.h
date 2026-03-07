/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    06.03.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::real_time_lighting::transparency_techniques::hybrid_alpha_blending
{
namespace constants
{
    // Shaders
    inline constexpr auto kGeometryVertexShaderFile = "geometry_pass.vert.spv";
    inline constexpr auto kGeometryFragmentShaderFile = "geometry_pass.frag.spv";
    inline constexpr auto kLightVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kLightFragmentShaderFile = "light_pass.frag.spv";
    inline constexpr auto kTransparencyFragmentShaderFile = "transparency_pass.frag.spv";
    inline constexpr auto kGeometryVertexShaderKey = "vertGeometry";
    inline constexpr auto kGeometryFragmentShaderKey = "fragGeometry";
    inline constexpr auto kLightVertexShaderKey = "vertLight";
    inline constexpr auto kLightFragmentShaderKey = "fragLight";
    inline constexpr auto kTransparencyFragmentShaderKey = "fragTransparency";

    // Buffers
    inline constexpr auto kLightStorageBuffer = "lightStorageBuffer";

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
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kLightDescSet = "lightDescSet";
    inline constexpr auto kLightDescSetLayout = "lightDescSetLayout";
    inline constexpr auto kTransparencyDescSet = "transparencyDescSet";
    inline constexpr auto kTransparencyDescSetLayout = "transparencyDescSetLayout";

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
    inline constexpr auto kTransparentPlane = "transparentPlane";

    // Scene Object Tags
    inline constexpr auto kTransparentObjectGroup = "transparentObjectGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto ConstantFactor = "AppSettings.ConstantFactor";
    constexpr auto LinearFactor = "AppSettings.LinearFactor";
    constexpr auto QuadraticFactor = "AppSettings.QuadraticFactor";
} // namespace AppSettings
} // namespace examples::real_time_lighting::transparency_techniques::hybrid_alpha_blending
