/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    07.11.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::post_processing_effects::blur_and_motion_effects::bokeh_depth_of_field
{
namespace constants
{
    // Shaders
    inline constexpr auto kGeometryVertexShaderFile = "geometry_pass.vert.spv";
    inline constexpr auto kGeometryFragmentShaderFile = "geometry_pass.frag.spv";
    inline constexpr auto kFullscreenVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kLightFragmentShaderFile = "light_pass.frag.spv";
    inline constexpr auto kDofPrefilterFragmentShaderFile = "dof_prefilter_pass.frag.spv";
    inline constexpr auto kDofGatherFragmentShaderFile = "dof_gather_pass.frag.spv";
    inline constexpr auto kDofPostfilterFragmentShaderFile = "dof_postfilter_pass.frag.spv";
    inline constexpr auto kBokehSpriteVertexShaderFile = "bokeh_sprite.vert.spv";
    inline constexpr auto kBokehSpriteFragmentShaderFile = "bokeh_sprite.frag.spv";
    inline constexpr auto kCompositeFragmentShaderFile = "dof_composite_pass.frag.spv";
    inline constexpr auto kGeometryVertexShaderKey = "vertGeometry";
    inline constexpr auto kGeometryFragmentShaderKey = "fragGeometry";
    inline constexpr auto kFullscreenVertexShaderKey = "vertFullscreen";
    inline constexpr auto kLightFragmentShaderKey = "fragLight";
    inline constexpr auto kDofPrefilterFragmentShaderKey = "fragDofPrefilter";
    inline constexpr auto kDofGatherFragmentShaderKey = "fragDofGather";
    inline constexpr auto kDofPostfilterFragmentShaderKey = "fragDofPostfilter";
    inline constexpr auto kBokehSpriteVertexShaderKey = "vertBokehSprite";
    inline constexpr auto kBokehSpriteFragmentShaderKey = "fragBokehSprite";
    inline constexpr auto kCompositeFragmentShaderKey = "fragComposite";

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
    inline constexpr auto kDofImageA = "dofImageA";
    inline constexpr auto kDofImageViewA = "dofImageViewA";
    inline constexpr auto kDofImageB = "dofImageB";
    inline constexpr auto kDofImageViewB = "dofImageViewB";
    inline constexpr auto kBokehSourceImage = "bokehSourceImage";
    inline constexpr auto kBokehSourceImageView = "bokehSourceImageView";
    inline constexpr auto kBokehImage = "bokehImage";
    inline constexpr auto kBokehImageView = "bokehImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kLinearClampSampler = "linearClampSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kLightDescSet = "lightDescSet";
    inline constexpr auto kLightDescSetLayout = "lightDescSetLayout";
    inline constexpr auto kDofPrefilterDescSet = "dofPrefilterDescSet";
    inline constexpr auto kDofPrefilterDescSetLayout = "dofPrefilterDescSetLayout";
    inline constexpr auto kDofImageDescSetA = "dofImageDescSetA";
    inline constexpr auto kDofImageDescSetB = "dofImageDescSetB";
    inline constexpr auto kDofSourceDescSetLayout = "dofSourceDescSetLayout";
    inline constexpr auto kBokehSpriteDescSet = "bokehSpriteDescSet";
    inline constexpr auto kBokehSpriteDescSetLayout = "bokehSpriteDescSetLayout";
    inline constexpr auto kCompositePassDescSet = "compositePassDescSet";
    inline constexpr auto kCompositePassDescSetLayout = "compositePassDescSetLayout";

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
} // namespace examples::post_processing_effects::blur_and_motion_effects::bokeh_depth_of_field
