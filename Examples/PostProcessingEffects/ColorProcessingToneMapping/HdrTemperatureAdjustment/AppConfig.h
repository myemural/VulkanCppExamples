/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    18.09.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::post_processing_effects::color_processing_tone_mapping::hdr_temperature_adjustment
{
namespace constants
{
    // Shaders
    inline constexpr auto kGeometryVertexShaderFile = "geometry_pass.vert.spv";
    inline constexpr auto kGeometryFragmentShaderFile = "geometry_pass.frag.spv";
    inline constexpr auto kFullscreenVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kLightFragmentShaderFile = "light_pass.frag.spv";
    inline constexpr auto kPostProcessingFragmentShaderFile = "temperature_and_tone_mapping_pass.frag.spv";
    inline constexpr auto kGeometryVertexShaderKey = "vertGeometry";
    inline constexpr auto kGeometryFragmentShaderKey = "fragGeometry";
    inline constexpr auto kFullscreenVertexShaderKey = "vertFullscreen";
    inline constexpr auto kLightFragmentShaderKey = "fragLight";
    inline constexpr auto kPostProcessingFragmentShaderKey = "fragPostProcessing";

    // Buffers
    inline constexpr auto kLightStorageBuffer = "lightStorageBuffer";

    // Images and Image Views
    inline constexpr auto kPositionImage = "positionImage";
    inline constexpr auto kPositionImageView = "positionImageView";
    inline constexpr auto kAlbedoImage = "albedoImage";
    inline constexpr auto kAlbedoImageView = "albedoImageView";
    inline constexpr auto kNormalImage = "normalImage";
    inline constexpr auto kNormalImageView = "normalImageView";
    inline constexpr auto kRoughnessMetallicImage = "roughnessMetallicImage";
    inline constexpr auto kRoughnessMetallicImageView = "roughnessMetallicImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";
    inline constexpr auto kLightingOutputImage = "lightingOutputImage";
    inline constexpr auto kLightingOutputImageView = "lightingOutputImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kLightDescSet = "lightDescSet";
    inline constexpr auto kLightDescSetLayout = "lightDescSetLayout";
    inline constexpr auto kPostProcessingDescSet = "postProcessingDescSet";
    inline constexpr auto kPostProcessingDescSetLayout = "postProcessingDescSetLayout";

    // Textures
    inline constexpr auto kFloorTexturePath = "Textures/Linoleum_Floor_001_basecolor.png";
    inline constexpr auto kFloorTexture = "floorTexture";
    inline constexpr auto kFloorNormalTexturePath = "Textures/Linoleum_Floor_001_normal.png";
    inline constexpr auto kFloorNormalTexture = "floorNormalTexture";
    inline constexpr auto kFloorRoughnessTexturePath = "Textures/Linoleum_Floor_001_roughness.png";
    inline constexpr auto kFloorRoughnessTexture = "floorRoughnessTexture";

    inline constexpr auto kMetalDamagedAlbedoTexturePath = "Textures/Metal_Damaged_001_basecolor.jpg";
    inline constexpr auto kMetalDamagedAlbedoTexture = "metalDamagedAlbedoTexture";
    inline constexpr auto kMetalDamagedRoughnessTexturePath = "Textures/Metal_Damaged_001_roughness.jpg";
    inline constexpr auto kMetalDamagedRoughnessTexture = "metalDamagedRoughnessTexture";
    inline constexpr auto kMetalDamagedMetallicTexturePath = "Textures/Metal_Damaged_001_metallic.jpg";
    inline constexpr auto kMetalDamagedMetallicTexture = "metalDamagedMetallicTexture";
    inline constexpr auto kMetalDamagedNormalTexturePath = "Textures/Metal_Damaged_001_normal.jpg";
    inline constexpr auto kMetalDamagedNormalTexture = "metalDamagedNormalTexture";

    // Materials
    inline constexpr auto kFloorMaterial = "floorMaterial";
    inline constexpr auto kGreenWallMaterial = "greenWallMaterial";
    inline constexpr auto kRedWallMaterial = "redWallMaterial";
    inline constexpr auto kWhiteWallMaterial = "whiteWallMaterial";
    inline constexpr auto kMetalDamagedMaterial = "metalDamagedMaterial";
    inline constexpr auto kRedMaterial = "redMaterial";
    inline constexpr auto kGreenMaterial = "greenMaterial";
    inline constexpr auto kBlueMaterial = "blueMaterial";
    inline constexpr auto kYellowMaterial = "yellowMaterial";
    inline constexpr auto kMagentaMaterial = "magentaMaterial";
    inline constexpr auto kCyanMaterial = "cyanMaterial";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kRoomObject = "room";
    inline constexpr auto kPropObject = "prop";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::post_processing_effects::color_processing_tone_mapping::hdr_temperature_adjustment
