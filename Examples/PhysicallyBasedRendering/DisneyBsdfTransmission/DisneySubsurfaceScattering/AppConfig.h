/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    20.10.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::physically_based_rendering::disney_bsdf_transmission::disney_subsurface_scattering
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "pbr.vert.spv";
    inline constexpr auto kSceneObjectsFragmentShaderFile = "pbr.frag.spv";
    inline constexpr auto kSkyboxVertexShaderFile = "skybox.vert.spv";
    inline constexpr auto kConvertToCubemapFragmentShaderFile = "convert_to_cubemap.frag.spv";
    inline constexpr auto kIrradianceConvolutionFragmentShaderFile = "irradiance_convolution.frag.spv";
    inline constexpr auto kBrdfLutGeneratorComputeShaderFile = "brdf_lut_generator.comp.spv";
    inline constexpr auto kPrefilteredCubemapComputeShaderFile = "prefiltered_map_generator.comp.spv";
    inline constexpr auto kSkyboxFragmentShaderFile = "skybox.frag.spv";
    inline constexpr auto kFullscreenQuadVertexShaderFile = "fullscreen_quad.vert.spv";
    inline constexpr auto kCompositeFragmentShaderFile = "composite.frag.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kSceneObjectsFragmentShaderKey = "fragScene";
    inline constexpr auto kSkyboxVertexShaderKey = "vertSkybox";
    inline constexpr auto kConvertToCubemapFragmentShaderKey = "fragConvertToCubemap";
    inline constexpr auto kIrradianceConvolutionFragmentShaderKey = "fragIrradianceConvolution";
    inline constexpr auto kSkyboxFragmentShaderKey = "fragSkybox";
    inline constexpr auto kBrdfLutGeneratorComputeShaderKey = "compBrdfLutGenerator";
    inline constexpr auto kPrefilteredCubemapComputeShaderKey = "compPrefilteredCubemap";
    inline constexpr auto kFullscreenQuadVertexShaderKey = "vertFullscreenQuad";
    inline constexpr auto kCompositeFragmentShaderKey = "fragComposite";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";
    inline constexpr auto kEnvironmentCubemapImage = "environmentCubemapImage";
    inline constexpr auto kEnvironmentCubemapImageView = "environmentCubemapImageView";
    inline constexpr auto kEnvironmentCubemapImageViewRight = "environmentCubemapImageViewRight";
    inline constexpr auto kEnvironmentCubemapImageViewLeft = "environmentCubemapImageViewLeft";
    inline constexpr auto kEnvironmentCubemapImageViewTop = "environmentCubemapImageViewTop";
    inline constexpr auto kEnvironmentCubemapImageViewBottom = "environmentCubemapImageViewBottom";
    inline constexpr auto kEnvironmentCubemapImageViewBack = "environmentCubemapImageViewBack";
    inline constexpr auto kEnvironmentCubemapImageViewFront = "environmentCubemapImageViewFront";
    inline constexpr auto kIrradianceCubemapImage = "irradianceCubemapImage";
    inline constexpr auto kIrradianceCubemapImageView = "irradianceCubemapImageView";
    inline constexpr auto kIrradianceCubemapImageViewRight = "irradianceCubemapImageViewRight";
    inline constexpr auto kIrradianceCubemapImageViewLeft = "irradianceCubemapImageViewLeft";
    inline constexpr auto kIrradianceCubemapImageViewTop = "irradianceCubemapImageViewTop";
    inline constexpr auto kIrradianceCubemapImageViewBottom = "irradianceCubemapImageViewBottom";
    inline constexpr auto kIrradianceCubemapImageViewBack = "irradianceCubemapImageViewBack";
    inline constexpr auto kIrradianceCubemapImageViewFront = "irradianceCubemapImageViewFront";
    inline constexpr auto kBrdfLutImage = "brdfLutImage";
    inline constexpr auto kBrdfLutImageView = "brdfLutImageView";
    inline constexpr auto kPrefilterCubemapImage = "prefilterCubemapImage";
    inline constexpr auto kPrefilterCubemapImageView = "prefilterCubemapImageView";
    inline constexpr auto kPrefilterCubemapImageViewMip = "prefilterCubemapImageViewMip";
    inline constexpr auto kSceneColorImage = "sceneColorImage";
    inline constexpr auto kSceneColorImageView = "sceneColorImageView";
    inline constexpr auto kSceneColorAttachmentImageView = "sceneColorAttachmentImageView";
    inline constexpr auto kSssIrradianceImage = "sssIrradianceImage";
    inline constexpr auto kSssIrradianceImageView = "sssIrradianceImageView";
    inline constexpr auto kSssAlbedoImage = "sssAlbedoImage";
    inline constexpr auto kSssAlbedoImageView = "sssAlbedoImageView";
    inline constexpr auto kSssScatterImage = "sssScatterImage";
    inline constexpr auto kSssScatterImageView = "sssScatterImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kMipmappedCubemapSampler = "mipmappedCubemapSampler";
    inline constexpr auto kIrradianceMapSampler = "irradianceMapSampler";
    inline constexpr auto kSceneColorSampler = "sceneColorSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainTransDescSet = "mainTransDescSet";
    inline constexpr auto kMainOpaqueDescSet = "mainOpaqueDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kConvertToCubemapDescSet = "convertToCubemapDescSet";
    inline constexpr auto kIrradianceConvolutionDescSet = "irradianceConvolutionDescSet";
    inline constexpr auto kSkyboxDescSet = "skyboxDescSet";
    inline constexpr auto kSkyboxDescSetLayout = "skyboxDescSetLayout";
    inline constexpr auto kBrdfLutDescSet = "brdfLutDescSet";
    inline constexpr auto kBrdfLutDescSetLayout = "brdfLutDescSetLayout";
    inline constexpr auto kPrefilterMapDescSet = "prefilterMapDescSet";
    inline constexpr auto kPrefilterMapDescSetLayout = "prefilterMapDescSetLayout";
    inline constexpr auto kCompositeDescSet = "compositeDescSet";
    inline constexpr auto kCompositeDescSetLayout = "compositeDescSetLayout";

    // Textures
    inline constexpr auto kHdrEnvironmentTexturePath = "Textures/sundowner_deck_2k.hdr";
    inline constexpr auto kHdrEnvironmentTexture = "hdrEnvironmentTexture";

    inline constexpr auto kFloorTexturePath = "Textures/Linoleum_Floor_001_basecolor.png";
    inline constexpr auto kFloorTexture = "floorTexture";
    inline constexpr auto kFloorNormalTexturePath = "Textures/Linoleum_Floor_001_normal.png";
    inline constexpr auto kFloorNormalTexture = "floorNormalTexture";
    inline constexpr auto kFloorRoughnessTexturePath = "Textures/Linoleum_Floor_001_roughness.png";
    inline constexpr auto kFloorRoughnessTexture = "floorRoughnessTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kSphereObject = "sphere";
    inline constexpr auto kBackPlane = "backPlane";
    inline constexpr auto kSkyboxCubeObject = "skyboxCube";

    // Models
    inline constexpr auto kSuzanneModelPath = "Models/SuzanneDetailed.glb";
    inline constexpr auto kSuzanneModelName = "suzanne";

    // Object Groups
    inline constexpr auto kSkyboxObjectGroup = "skyboxObjectGroup";
    inline constexpr auto kTransparentObjectGroup = "transparentObjectGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::physically_based_rendering::disney_bsdf_transmission::disney_subsurface_scattering
