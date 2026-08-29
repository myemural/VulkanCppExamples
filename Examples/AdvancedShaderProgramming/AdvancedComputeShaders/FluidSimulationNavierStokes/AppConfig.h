/**
 * @file    AppConfig.h
 * @brief   This header file keeps example specific constants and settings.
 * @author  Mustafa Yemural (myemural)
 * @date    29.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_navier_stokes
{
namespace constants
{
    // Shaders
    inline constexpr auto kSceneVertexShaderFile = "scene.vert.spv";
    inline constexpr auto kSceneFragmentShaderFile = "scene.frag.spv";
    inline constexpr auto kSmokeFragmentShaderFile = "smoke.frag.spv";
    inline constexpr auto kAdvectComputeShaderFile = "fluid_advect.comp.spv";
    inline constexpr auto kJacobiComputeShaderFile = "fluid_jacobi.comp.spv";
    inline constexpr auto kProjectComputeShaderFile = "fluid_project.comp.spv";
    inline constexpr auto kSceneVertexShaderKey = "vertScene";
    inline constexpr auto kSceneFragmentShaderKey = "fragScene";
    inline constexpr auto kSmokeFragmentShaderKey = "fragSmoke";
    inline constexpr auto kAdvectComputeShaderKey = "compAdvect";
    inline constexpr auto kJacobiComputeShaderKey = "compJacobi";
    inline constexpr auto kProjectComputeShaderKey = "compProject";

    // Buffers
    inline constexpr auto kSceneUniformBuffer = "sceneUniformBuffer";

    // Images and Image Views
    inline constexpr auto kDepthImage = "depthImage";
    inline constexpr auto kDepthImageView = "depthImageView";
    inline constexpr auto kFieldImageA = "fieldImageA";
    inline constexpr auto kFieldImageViewA = "fieldImageViewA";
    inline constexpr auto kFieldImageB = "fieldImageB";
    inline constexpr auto kFieldImageViewB = "fieldImageViewB";
    inline constexpr auto kPressureImageA = "pressureImageA";
    inline constexpr auto kPressureImageViewA = "pressureImageViewA";
    inline constexpr auto kPressureImageB = "pressureImageB";
    inline constexpr auto kPressureImageViewB = "pressureImageViewB";

    // Samplers
    inline constexpr auto kVolumeSampler = "volumeSampler";

    // Descriptor Sets and Layouts
    inline constexpr auto kMainDescSet = "mainDescSet";
    inline constexpr auto kMainDescSetLayout = "mainDescSetLayout";
    inline constexpr auto kAdvectDescSet = "advectDescSet";
    inline constexpr auto kAdvectDescSetLayout = "advectDescSetLayout";
    inline constexpr auto kJacobiDescSetAToB = "jacobiDescSetAtoB";
    inline constexpr auto kJacobiDescSetBToA = "jacobiDescSetBtoA";
    inline constexpr auto kJacobiDescSetLayout = "jacobiDescSetLayout";
    inline constexpr auto kProjectDescSet = "projectDescSet";
    inline constexpr auto kProjectDescSetLayout = "projectDescSetLayout";

    // Scene Objects
    inline constexpr auto kRootObject = "root";
    inline constexpr auto kGroundPlane = "groundPlane";
    inline constexpr auto kSmokeVolume = "smokeVolume";

    // Object groups
    inline constexpr auto kSmokeGroup = "smokeGroup";
} // namespace constants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraZoomSpeed = "AppSettings.CameraZoomSpeed";
} // namespace AppSettings
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_navier_stokes
