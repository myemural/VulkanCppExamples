/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    24.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::advanced_compute_shaders::cloth_simulation_compute
{
namespace constants
{
    // Shaders
    inline constexpr auto kSceneVertexShaderFile = "scene.vert.spv";
    inline constexpr auto kSceneFragmentShaderFile = "scene.frag.spv";
    inline constexpr auto kClothVertexShaderFile = "cloth.vert.spv";
    inline constexpr auto kClothFragmentShaderFile = "cloth.frag.spv";
    inline constexpr auto kClothIntegrationComputeShaderFile = "cloth_integration.comp.spv";
    inline constexpr auto kClothConstraintComputeShaderFile = "cloth_constraint.comp.spv";
    inline constexpr auto kClothUpdateComputeShaderFile = "cloth_update.comp.spv";
    inline constexpr auto kSceneVertexShaderKey = "vertScene";
    inline constexpr auto kSceneFragmentShaderKey = "fragScene";
    inline constexpr auto kClothVertexShaderKey = "vertCloth";
    inline constexpr auto kClothFragmentShaderKey = "fragCloth";
    inline constexpr auto kClothIntegrationComputeShaderKey = "compClothIntegration";
    inline constexpr auto kClothConstraintComputeShaderKey = "compClothConstraint";
    inline constexpr auto kClothUpdateComputeShaderKey = "compClothUpdate";

    // Buffers
    inline constexpr auto kLightUniformBuffer = "lightUniformBuffer";
    inline constexpr auto kClothUniformBuffer = "clothUniformBuffer";
    inline constexpr auto kClothParticleBufferA = "clothParticleBufferA";
    inline constexpr auto kClothParticleBufferB = "clothParticleBufferB";
    inline constexpr auto kClothVertexBuffer = "clothVertexBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";

    // Samplers
    inline constexpr auto kMainSampler = "mainSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kClothDescriptorSetPing = "clothDescriptorSetPing";
    inline constexpr auto kClothDescriptorSetPong = "clothDescriptorSetPong";
    inline constexpr auto kClothDescriptorSetLayout = "clothDescriptorSetLayout";

    // Textures
    inline constexpr auto kWallStoneTexturePath = "Textures/Wall_Stone_034_basecolor.png";
    inline constexpr auto kWallStoneTexture = "wallStoneTexture";
    inline constexpr auto kWallStoneNormalTexturePath = "Textures/Wall_Stone_034_normal.png";
    inline constexpr auto kWallStoneNormalTexture = "wallStoneNormalTexture";

    inline constexpr auto kFabricDenimPatchTexturePath = "Textures/Fabric_Denim_Patch_001_basecolor.png";
    inline constexpr auto kFabricDenimPatchTexture = "fabricDenimPatchTexture";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kSphereObject = "sphere";
    inline constexpr auto kGroundObject = "ground";
    inline constexpr auto kClothObject = "cloth";

    // Object groups
    inline constexpr auto kClothTag = "clothGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::cloth_simulation_compute
