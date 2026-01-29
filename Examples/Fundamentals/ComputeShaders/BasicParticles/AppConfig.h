/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
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
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "position_with_mvp.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "texture_sampler.frag.spv";
    inline constexpr auto kParticleVertexShaderFile = "particle_position.vert.spv";
    inline constexpr auto kParticleFragmentShaderFile = "particle_coloring.frag.spv";
    inline constexpr auto kParticleComputeShaderFile = "particle_generator.comp.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";
    inline constexpr auto kParticleVertexShaderKey = "vertParticle";
    inline constexpr auto kParticleFragmentShaderKey = "fragParticle";
    inline constexpr auto kParticleComputeShaderKey = "compParticle";

    // Buffers
    inline constexpr auto kCubeVertexBuffer = "cubeVertexBuffer";
    inline constexpr auto kCubeIndexBuffer = "cubeIndexBuffer";
    inline constexpr auto kParticleStorageBuffer = "particleStorageBuffer";

    // Images and Image Views
    inline constexpr auto kCrateImage = "crateImage";
    inline constexpr auto kCrateImageView = "crateImageView";
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kCubeDescSet = "cubeDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kParticleDescSetLayout = "particleDescSetLayout";
    inline constexpr auto kParticleComputeDescSetLayout = "particleComputeDescSetLayout";

    // Textures
    inline constexpr auto kCrateTexturePath = "Textures/crate1_diffuse.png";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
    constexpr auto ParticleCount = "AppSettings.ParticleCount";
} // namespace AppSettings
} // namespace examples::fundamentals::compute_shaders::basic_particles
