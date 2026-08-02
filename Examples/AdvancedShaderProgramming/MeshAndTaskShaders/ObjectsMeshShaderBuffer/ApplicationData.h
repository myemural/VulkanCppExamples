/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    02.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::advanced_shader_programming::mesh_and_task_shaders::objects_mesh_shader_buffer
{

// Constants
inline constexpr auto kTrianglesPerWorkgroup = 64U;
inline constexpr auto kObjectCount = 100;
inline constexpr auto kDefaultBufferSizeInBytes = 100'000UL; // 100 KB
inline constexpr auto kLightDirection = glm::vec3(-0.4f, -0.4f, -0.4f);
inline constexpr auto kLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

struct GpuMesh
{
    std::uint32_t vertexOffset = 0;   // First index for the position and normal buffers
    std::uint32_t vertexCount = 0;
    std::uint32_t indexOffset = 0;    // First index for the index buffer
    std::uint32_t primitiveCount = 0; // Number of triangles = indexCount / 3
};

struct MeshPushConstants
{
    glm::mat4 model;
    glm::mat4 invModel;
    std::uint32_t vertexOffset;   // Offset into the global vertex buffers
    std::uint32_t vertexCount;    // Number of vertices belonging to this mesh
    std::uint32_t indexOffset;    // Offset into the global index buffer
    std::uint32_t primitiveCount; // Number of triangles in this mesh
};

struct alignas(16) SceneUbo
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPosition;
    glm::vec4 lightDirection;
    glm::vec4 lightColor;
    glm::vec4 diffuseColor;
    glm::vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
};

} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::objects_mesh_shader_buffer
