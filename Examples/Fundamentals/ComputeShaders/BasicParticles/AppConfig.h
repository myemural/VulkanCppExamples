/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    13.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::compute_shaders::basic_particles
{
namespace AppConstants
{
    // Shaders
    constexpr auto BaseShaderType = "AppConstants.BaseShaderType";
    constexpr auto MainVertexShaderFile = "AppConstants.MainVertexShaderFile";
    constexpr auto MainFragmentShaderFile = "AppConstants.MainFragmentShaderFile";
    constexpr auto ParticleVertexShaderFile = "AppConstants.ParticleVertexShaderFile";
    constexpr auto ParticleFragmentShaderFile = "AppConstants.ParticleFragmentShaderFile";
    constexpr auto ParticleComputeShaderFile = "AppConstants.ParticleComputeShaderFile";
    constexpr auto MainVertexShaderKey = "AppConstants.MainVertexShaderKey";
    constexpr auto MainFragmentShaderKey = "AppConstants.MainFragmentShaderKey";
    constexpr auto ParticleVertexShaderKey = "AppConstants.ParticleVertexShaderKey";
    constexpr auto ParticleFragmentShaderKey = "AppConstants.ParticleFragmentShaderKey";
    constexpr auto ParticleComputeShaderKey = "AppConstants.ParticleComputeShaderKey";

    // Resources
    constexpr auto CubeVertexBuffer = "AppConstants.CubeVertexBuffer";
    constexpr auto CubeIndexBuffer = "AppConstants.CubeIndexBuffer";
    constexpr auto ParticleStorageBuffer = "AppConstants.ParticleStorageBuffer";
    constexpr auto CrateImage = "AppConstants.CrateImage";
    constexpr auto CrateImageView = "AppConstants.CrateImageView";
    constexpr auto DepthImage = "AppConstants.DepthImage";
    constexpr auto DepthImageView = "AppConstants.DepthImageView";
    constexpr auto MainSampler = "AppConstants.MainSampler";
    constexpr auto CubeDescSet = "AppConstants.CubeDescSet";
    constexpr auto MainDescSetLayout = "AppConstants.MainDescSetLayout";
    constexpr auto ParticleDescSetLayout = "AppConstants.ParticleDescSetLayout";
    constexpr auto ParticleComputeDescSetLayout = "AppConstants.ParticleComputeDescSetLayout";
    constexpr auto CrateTexturePath = "AppConstants.CrateTexturePath";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto ParticleCount = "AppSettings.ParticleCount";
} // namespace AppSettings
} // namespace examples::fundamentals::compute_shaders::basic_particles
