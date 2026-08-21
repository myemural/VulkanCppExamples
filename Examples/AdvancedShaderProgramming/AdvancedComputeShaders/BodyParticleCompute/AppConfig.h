/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    21.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::advanced_compute_shaders::body_particle_compute
{
namespace constants
{
    // Shaders
    inline constexpr auto kMainVertexShaderFile = "particle_sprite.vert.spv";
    inline constexpr auto kMainFragmentShaderFile = "particle_sprite.frag.spv";
    inline constexpr auto kNBodyComputeShaderFile = "nbody.comp.spv";
    inline constexpr auto kMainVertexShaderKey = "vertMain";
    inline constexpr auto kMainFragmentShaderKey = "fragMain";
    inline constexpr auto kNBodyComputeShaderKey = "compNBody";

    // Buffers
    inline constexpr auto kParticleBufferA = "bufferParticleA";
    inline constexpr auto kParticleBufferB = "bufferParticleB";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSetA = "mainDescSetA";
    inline constexpr auto kMainDescSetB = "mainDescSetB";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kComputeDescSetAToB = "computeDescSetAToB";
    inline constexpr auto kComputeDescSetBToA = "computeDescSetBToA";
    inline constexpr auto kComputeDescSetLayout = "computeDescSetLayout";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::body_particle_compute
