/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    11.11.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::post_processing_effects::blur_and_motion_effects::per_object_motion_blur
{
namespace constants
{
    // Shaders
    inline constexpr auto kGeometryVertexShaderFile = "geometry_pass.vert.spv";
    inline constexpr auto kGeometryFragmentShaderFile = "geometry_pass.frag.spv";
    inline constexpr auto kFullscreenVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kLightFragmentShaderFile = "light_pass.frag.spv";
    inline constexpr auto kVelocityTileMaxFragmentShaderFile = "velocity_tile_max_pass.frag.spv";
    inline constexpr auto kVelocityNeighborMaxFragmentShaderFile = "velocity_neighbor_max_pass.frag.spv";
    inline constexpr auto kPostProcessingFragmentShaderFile = "per_object_motion_blur_pass.frag.spv";
    inline constexpr auto kGeometryVertexShaderKey = "vertGeometry";
    inline constexpr auto kGeometryFragmentShaderKey = "fragGeometry";
    inline constexpr auto kFullscreenVertexShaderKey = "vertFullscreen";
    inline constexpr auto kLightFragmentShaderKey = "fragLight";
    inline constexpr auto kVelocityTileMaxFragmentShaderKey = "fragVelocityTileMax";
    inline constexpr auto kVelocityNeighborMaxFragmentShaderKey = "fragVelocityNeighborMax";
    inline constexpr auto kPostProcessingFragmentShaderKey = "fragPostProcessing";

    // Buffers
    inline constexpr auto kLightStorageBuffer = "lightStorageBuffer";
    inline constexpr auto kPreviousFrameDataBuffer = "previousFrameDataBuffer";

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
    inline constexpr auto kVelocityImage = "velocityImage";
    inline constexpr auto kVelocityImageView = "velocityImageView";
    inline constexpr auto kTileMaxImage = "tileMaxImage";
    inline constexpr auto kTileMaxImageView = "tileMaxImageView";
    inline constexpr auto kNeighborMaxImage = "neighborMaxImage";
    inline constexpr auto kNeighborMaxImageView = "neighborMaxImageView";
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
    inline constexpr auto kTileMaxDescSet = "tileMaxDescSet";
    inline constexpr auto kNeighborMaxDescSet = "neighborMaxDescSet";
    inline constexpr auto kVelocitySourceDescSetLayout = "velocitySourceDescSetLayout";

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
    inline constexpr auto kMovingObject = "moving";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::post_processing_effects::blur_and_motion_effects::per_object_motion_blur
