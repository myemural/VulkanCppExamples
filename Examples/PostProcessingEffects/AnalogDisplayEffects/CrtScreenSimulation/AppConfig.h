/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    31.10.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::post_processing_effects::analog_display_effects::crt_screen_simulation
{
namespace constants
{
    // Shaders
    inline constexpr auto kGeometryVertexShaderFile = "geometry_pass.vert.spv";
    inline constexpr auto kGeometryFragmentShaderFile = "geometry_pass.frag.spv";
    inline constexpr auto kFullscreenVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kLightFragmentShaderFile = "light_pass.frag.spv";
    inline constexpr auto kVirtualImageFragmentShaderFile = "crt_virtual_image_pass.frag.spv";
    inline constexpr auto kBlurFragmentShaderFile = "gaussian_blur_pass.frag.spv";
    inline constexpr auto kCrtFragmentShaderFile = "crt_screen_pass.frag.spv";
    inline constexpr auto kGeometryVertexShaderKey = "vertGeometry";
    inline constexpr auto kGeometryFragmentShaderKey = "fragGeometry";
    inline constexpr auto kFullscreenVertexShaderKey = "vertFullscreen";
    inline constexpr auto kLightFragmentShaderKey = "fragLight";
    inline constexpr auto kVirtualImageFragmentShaderKey = "fragVirtualImage";
    inline constexpr auto kBlurFragmentShaderKey = "fragBlurPass";
    inline constexpr auto kCrtFragmentShaderKey = "fragCrt";

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
    inline constexpr auto kVirtualImage = "virtualImage";
    inline constexpr auto kVirtualImageView = "virtualImageView";
    inline constexpr auto kHalationImageA = "halationImageA";
    inline constexpr auto kHalationImageViewA = "halationImageViewA";
    inline constexpr auto kHalationImageB = "halationImageB";
    inline constexpr auto kHalationImageViewB = "halationImageViewB";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kLinearClampSampler = "linearClampSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kLightDescSet = "lightDescSet";
    inline constexpr auto kLightDescSetLayout = "lightDescSetLayout";
    inline constexpr auto kVirtualImageDescSet = "virtualImageDescSet";
    inline constexpr auto kVirtualImageDescSetLayout = "virtualImageDescSetLayout";
    inline constexpr auto kBlurHorizontalDescSet = "blurHorizontalDescSet";
    inline constexpr auto kBlurVerticalDescSet = "blurVerticalDescSet";
    inline constexpr auto kBlurDescSetLayout = "blurDescSetLayout";
    inline constexpr auto kCrtDescSet = "crtDescSet";
    inline constexpr auto kCrtDescSetLayout = "crtDescSetLayout";

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
    inline constexpr auto kLightObjectMaterial = "lightObjectMaterial";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kRoomObject = "room";
    inline constexpr auto kPropObject = "prop";
    inline constexpr auto kLightSphereObject = "lightSphere";

    // Object Groups
    inline constexpr auto kLightObjectGroup = "lightObjectGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::post_processing_effects::analog_display_effects::crt_screen_simulation
