/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    27.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::advanced_shader_programming::advanced_compute_shaders::fft_ocean_compute
{

inline const std::vector enabledMaterialComponents{
    common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4,      common::scene::MaterialComponent::SPECULAR_COLOR_VEC4,
    common::scene::MaterialComponent::AMBIENT_STRENGTH_FLOAT,  common::scene::MaterialComponent::SHININESS_FLOAT,
    common::scene::MaterialComponent::SPECULAR_STRENGTH_FLOAT, common::scene::MaterialComponent::UV_SCALE_FLOAT,
    common::scene::MaterialComponent::DIFFUSE_MAP_TEXTURE,     common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

inline const std::vector attributeLayouts{
    std::pair(common::scene::AttributeType::POSITION, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TEXCOORD, common::scene::AccessorType::VEC2),
    std::pair(common::scene::AttributeType::NORMAL, common::scene::AccessorType::VEC3),
    std::pair(common::scene::AttributeType::TANGENT, common::scene::AccessorType::VEC4)};

// Constants
inline constexpr auto kLightColor = glm::vec3(1.0f, 0.96f, 0.88f);
inline constexpr auto kLightDirection = glm::vec3(-0.35f, -0.3f, 0.88f);
inline constexpr auto kOceanUvScale = 50.0f;
inline constexpr auto kFogStartDistance = 130.0f;
inline constexpr auto kFogEndDistance = 200.0f;
inline constexpr auto kCameraStartPosition = glm::vec3(0.0f, 8.0f, 60.0f);
inline constexpr auto kCameraFov = 55.0f;
inline constexpr auto kCameraNearPlane = 0.1f;
inline constexpr auto kCameraFarPlane = 600.0f;

inline constexpr auto kFftSize = 256U;
inline constexpr auto kFftLog2Size = 8U;              // log2(kFftSize), must match with the Ocean FFT Compute Shader
inline constexpr auto kOceanGridSize = kFftSize - 1U; // Quad count per side
inline constexpr auto kOceanVertexCount = kFftSize * kFftSize;
inline constexpr auto kOceanLocalSize = 16U;          // Compute (spectrum, displacement) shader local size X and Y
inline constexpr auto kOceanGroupCount = kFftSize / kOceanLocalSize;
inline constexpr auto kOceanPlaneSize = 200.0f;
inline constexpr auto kOceanPatchLength =
        kOceanPlaneSize * static_cast<float>(kFftSize) / static_cast<float>(kOceanGridSize);
inline constexpr auto kOceanLevel = 0.0f;

inline constexpr auto kGravity = 9.81f;
inline constexpr auto kWindSpeed = 15.0f;           // meter/seconds, drives the dominant wavelength
inline constexpr auto kPhillipsAmplitude = 1.0e-6f; // Overall wave energy
inline constexpr auto kSmallWaveSuppression = 0.8f; // meter, cuts waves shorter than this length
inline constexpr auto kOppositeWaveDamping = 0.07f; // Energy of waves traveling against the wind
inline constexpr auto kChoppiness = 1.2f;           // Horizontal displacement strength
inline constexpr auto kAmplitudeScale = 1.0f;       // Extra height multiplier for tweaking
inline constexpr auto kTimeScale = 1.0f;
inline const auto kWindDirection = glm::normalize(glm::vec2(1.0f, 0.35f));
inline constexpr auto kFoamThreshold = 0.85f;
inline constexpr auto kFoamSharpness = 1.6f;
inline constexpr auto kFoamIntensity = 0.9f;

struct alignas(16) LightUbo
{
    glm::vec4 lightDirection; // xyz = Light Direction
    glm::vec4 lightColor;     // xyz = Light Color
    glm::vec4 sceneParams;    // x = Time, y = Foam Intensity, z = Fog Start, w = Fog End
};

struct alignas(16) OceanUbo
{
    glm::vec4 patchParams; // x = Patch Length, y = Gravity, z = Time, w = Choppiness
    glm::vec4 waveParams;  // x = Amplitude Scale, y = Foam Threshold, z = Foam Sharpness
    glm::uvec4 gridInfo;   // x = FFT Size
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
};

struct FftPushConstants
{
    std::uint32_t direction; // 0 = Rows (horizontal), 1 = Columns (vertical)
};

struct OceanVertex
{
    glm::vec4 displacement; // xyz = World-Space Displacement, w = Foam Factor
    glm::vec4 normal;       // xyz = World-Space Normal
};

} // namespace examples::advanced_shader_programming::advanced_compute_shaders::fft_ocean_compute
