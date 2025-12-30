/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided config key names.
 * @author  Mustafa Yemural (myemural)
 * @date    2.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::multisampling::explicit_resolving
{
namespace AppConstants
{
    // Shaders
    constexpr auto QuadVertexShaderFile = "AppConstants.QuadVertexShaderFile";
    constexpr auto SceneVertexShaderFile = "AppConstants.SceneVertexShaderFile";
    constexpr auto MainFragmentShaderFile = "AppConstants.MainFragmentShaderFile";
    constexpr auto QuadVertexShaderKey = "AppConstants.QuadVertexShaderKey";
    constexpr auto SceneVertexShaderKey = "AppConstants.SceneVertexShaderKey";
    constexpr auto MainFragmentShaderKey = "AppConstants.MainFragmentShaderKey";

    // Resources
    constexpr auto SphereVertexBuffer = "AppConstants.SphereVertexBuffer";
    constexpr auto SphereIndexBuffer = "AppConstants.SphereIndexBuffer";
    constexpr auto PlaneVertexBuffer = "AppConstants.PlaneVertexBuffer";
    constexpr auto PlaneIndexBuffer = "AppConstants.PlaneIndexBuffer";
    constexpr auto MarbleImage = "AppConstants.MarbleImage";
    constexpr auto MarbleImageView = "AppConstants.MarbleImageView";
    constexpr auto MultisampledImage = "AppConstants.MultisampledImage";
    constexpr auto MultisampledImageView = "AppConstants.MultisampledImageView";
    constexpr auto ResolvedImage = "AppConstants.ResolvedImage";
    constexpr auto ResolvedImageView = "AppConstants.ResolvedImageView";
    constexpr auto DepthImage = "AppConstants.DepthImage";
    constexpr auto DepthImageView = "AppConstants.DepthImageView";
    constexpr auto MainSampler = "AppConstants.MainSampler";
    constexpr auto SceneDescSetLayout = "AppConstants.SceneDescSetLayout";
    constexpr auto QuadDescSetLayout = "AppConstants.QuadDescSetLayout";
    constexpr auto MarbleTexturePath = "AppConstants.MarbleTexturePath";
} // namespace AppConstants

namespace AppSettings
{
    constexpr auto ClearColor = "AppSettings.ClearColor";
    constexpr auto MouseSensitivity = "AppSettings.MouseSensitivity";
    constexpr auto CameraSpeed = "AppSettings.CameraSpeed";
} // namespace AppSettings
} // namespace examples::fundamentals::multisampling::explicit_resolving
