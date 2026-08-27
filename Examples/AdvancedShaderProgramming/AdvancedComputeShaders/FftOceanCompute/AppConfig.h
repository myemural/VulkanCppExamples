/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    27.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::advanced_compute_shaders::fft_ocean_compute
{
namespace constants
{
    // Shaders
    inline constexpr auto kOceanVertexShaderFile = "ocean.vert.spv";
    inline constexpr auto kOceanFragmentShaderFile = "ocean.frag.spv";
    inline constexpr auto kSkyboxVertexShaderFile = "skybox.vert.spv";
    inline constexpr auto kSkyboxFragmentShaderFile = "skybox.frag.spv";
    inline constexpr auto kSpectrumComputeShaderFile = "ocean_spectrum.comp.spv";
    inline constexpr auto kFftComputeShaderFile = "ocean_fft.comp.spv";
    inline constexpr auto kDisplacementComputeShaderFile = "ocean_displacement.comp.spv";
    inline constexpr auto kOceanVertexShaderKey = "vertOcean";
    inline constexpr auto kOceanFragmentShaderKey = "fragOcean";
    inline constexpr auto kSkyboxVertexShaderKey = "vertSkybox";
    inline constexpr auto kSkyboxFragmentShaderKey = "fragSkybox";
    inline constexpr auto kSpectrumComputeShaderKey = "compSpectrum";
    inline constexpr auto kFftComputeShaderKey = "compFft";
    inline constexpr auto kDisplacementComputeShaderKey = "compDisplacement";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";
    inline constexpr auto kOceanUniformBuffer = "oceanUniformBuffer";
    inline constexpr auto kInitialSpectrumBuffer = "initialSpectrumBuffer";
    inline constexpr auto kSpectrumBuffer = "spectrumBuffer";
    inline constexpr auto kOceanVertexBuffer = "oceanVertexBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";
    inline constexpr auto kSkyboxSampler = "skyboxSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kOceanComputeDescSet = "oceanComputeDescSet";
    inline constexpr auto kOceanComputeDescSetLayout = "oceanComputeDescSetLayout";

    // Textures
    inline constexpr auto kWaterNormalTexturePath = "Textures/Water_002_NORM.jpg";
    inline constexpr auto kWaterNormalTexture = "waterNormalTexture";

    inline constexpr auto kCubemapRightTexturePath = "Textures/calm_sea_cubemap/calm_sea_right.jpg";
    inline constexpr auto kCubemapLeftTexturePath = "Textures/calm_sea_cubemap/calm_sea_left.jpg";
    inline constexpr auto kCubemapTopTexturePath = "Textures/calm_sea_cubemap/calm_sea_up.jpg";
    inline constexpr auto kCubemapBottomTexturePath = "Textures/calm_sea_cubemap/calm_sea_down.jpg";
    inline constexpr auto kCubemapBackTexturePath = "Textures/calm_sea_cubemap/calm_sea_back.jpg";
    inline constexpr auto kCubemapFrontTexturePath = "Textures/calm_sea_cubemap/calm_sea_front.jpg";
    inline constexpr auto kCubemapTexture = "cubemapTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kOceanObject = "ocean";
    inline constexpr auto kSkyboxCubeObject = "skyboxCube";

    // Object groups
    inline constexpr auto kSkyboxObjectGroup = "skyboxObjectGroup";
    inline constexpr auto kOceanObjectGroup = "oceanObjectGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::fft_ocean_compute
