/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    06.06.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::physically_based_rendering::disney_principled_brdf::disney_diffuse_specular
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
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kSceneObjectsFragmentShaderKey = "fragScene";
    inline constexpr auto kSkyboxVertexShaderKey = "vertSkybox";
    inline constexpr auto kConvertToCubemapFragmentShaderKey = "fragConvertToCubemap";
    inline constexpr auto kIrradianceConvolutionFragmentShaderKey = "fragIrradianceConvolution";
    inline constexpr auto kSkyboxFragmentShaderKey = "fragSkybox";
    inline constexpr auto kBrdfLutGeneratorComputeShaderKey = "compBrdfLutGenerator";
    inline constexpr auto kPrefilteredCubemapComputeShaderKey = "compPrefilteredCubemap";

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

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kMipmappedCubemapSampler = "mipmappedCubemapSampler";
    inline constexpr auto kIrradianceMapSampler = "irradianceMapSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kConvertToCubemapDescSet = "convertToCubemapDescSet";
    inline constexpr auto kIrradianceConvolutionDescSet = "irradianceConvolutionDescSet";
    inline constexpr auto kSkyboxDescSet = "skyboxDescSet";
    inline constexpr auto kSkyboxDescSetLayout = "skyboxDescSetLayout";
    inline constexpr auto kBrdfLutDescSet = "brdfLutDescSet";
    inline constexpr auto kBrdfLutDescSetLayout = "brdfLutDescSetLayout";
    inline constexpr auto kPrefilterMapDescSet = "prefilterMapDescSet";
    inline constexpr auto kPrefilterMapDescSetLayout = "prefilterMapDescSetLayout";

    // Textures
    inline constexpr auto kHdrEnvironmentTexturePath = "Textures/sundowner_deck_2k.hdr";
    inline constexpr auto kHdrEnvironmentTexture = "hdrEnvironmentTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kSphereObject = "sphere";
    inline constexpr auto kSkyboxCubeObject = "skyboxCube";

    // Object Groups
    inline constexpr auto kSkyboxObjectGroup = "skyboxObjectGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::physically_based_rendering::disney_principled_brdf::disney_diffuse_specular
