/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    28.11.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "MathUtils.h"
#include "Vertex.h"

namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::ray_traced_triangle
{

// Vertex Data (world space positions, the triangle lies on the z = 0 plane)
// clang-format off
inline const std::vector<common::utility::VertexPos3> vertices {
    {glm::vec3{ 0.0f,  0.6f, 0.0f}}, // Top          (red in the closest hit shader)
    {glm::vec3{-0.6f, -0.6f, 0.0f}}, // Bottom-left  (green in the closest hit shader)
    {glm::vec3{ 0.6f, -0.6f, 0.0f}}  // Bottom-right (blue in the closest hit shader)
};
// clang-format on

// Index Data
inline const std::vector<std::uint32_t> indices{0, 1, 2};

// Constants
inline constexpr auto kCameraStartPosition = glm::vec3(0.0f, 0.0f, 2.5f);
inline constexpr auto kRadianceImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

// Shader stage indices in the ray tracing pipeline (in the order they are added)
inline constexpr std::uint32_t kRayGenShaderIndex = 0;
inline constexpr std::uint32_t kMissShaderIndex = 1;
inline constexpr std::uint32_t kClosestHitShaderIndex = 2;

// Shader group indices in the ray tracing pipeline (in the order they are added)
inline constexpr std::uint32_t kRayGenGroupIndex = 0;
inline constexpr std::uint32_t kMissGroupIndex = 1;
inline constexpr std::uint32_t kHitGroupIndex = 2;
inline constexpr std::uint32_t kShaderGroupCount = 3;

struct alignas(16) SceneUbo
{
    glm::mat4 viewInverse; // Transforms camera-space to world-space
    glm::mat4 projInverse; // Transforms clip-space to camera-space
    glm::vec4 missColor;   // rgb = Background color of the missed rays
};

} // namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::ray_traced_triangle
