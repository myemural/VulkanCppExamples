/**
 * @file    PrimitiveUtils.h
 * @brief   This file contains parametric generation functions for the primitives.
 * @author  Mustafa Yemural (myemural)
 * @date    3.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <numbers>
#include <vector>

#include "Vertex.h"

namespace common::utility
{

/**
 * @brief Calculates and returns vertices of a sphere.
 * @tparam VertexType Structure of a vertex.
 * @param radius Radius of the sphere.
 * @param sectorCount Sector count of the sphere.
 * @param stackCount Stack count of the sphere.
 * @return Vertices for sphere.
 */
template<typename VertexType>
std::vector<VertexType> CreateSphereVertices(float radius, uint32_t sectorCount, uint32_t stackCount);

template<>
inline std::vector<VertexPos3Uv2>
CreateSphereVertices(const float radius, const uint32_t sectorCount, const uint32_t stackCount)
{
    std::vector<VertexPos3Uv2> vertices;

    for (uint32_t i = 0; i <= stackCount; ++i) {
        const auto stackAngle = static_cast<float>(std::numbers::pi / 2.0 - i * std::numbers::pi / stackCount);
        const float xy = radius * cosf(stackAngle);
        const float z = radius * sinf(stackAngle);

        for (uint32_t j = 0; j <= sectorCount; ++j) {
            const auto sectorAngle = static_cast<float>(j * 2 * std::numbers::pi / sectorCount);

            const float x = xy * cosf(sectorAngle);
            const float y = xy * sinf(sectorAngle);

            const float u = static_cast<float>(j) / static_cast<float>(sectorCount);
            const float v = static_cast<float>(i) / static_cast<float>(stackCount);

            vertices.push_back({glm::vec3{x, y, z}, glm::vec2{u, v}});
        }
    }

    return vertices;
}

/**
 * @brief Calculates and returns indices of a sphere.
 * @param sectorCount Sector count of the sphere.
 * @param stackCount Stack count of the sphere.
 * @return Indices of the sphere.
 */
static std::vector<uint16_t> CreateSphereIndices(const uint32_t sectorCount, const uint32_t stackCount)
{
    std::vector<uint16_t> indices;

    for (uint32_t i = 0; i < stackCount; ++i) {
        uint32_t k1 = i * (sectorCount + 1);
        uint32_t k2 = k1 + sectorCount + 1;

        for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    return indices;
}

/**
 * @brief Calculates and returns vertices of a cube.
 * @tparam VertexType Structure of a vertex.
 * @param size Size of the cube.
 * @return Vertices of the cube.
 */
template<typename VertexType>
std::vector<VertexType> CreateCubeVertices(float size);

template<>
inline std::vector<VertexPos3Uv2> CreateCubeVertices(const float size)
{
    const float h = size * 0.5f;

    return {
        // Front (Z+)
        {glm::vec3{-h, -h, h}, glm::vec2{0, 0}},
        {glm::vec3{h, -h, h}, glm::vec2{1, 0}},
        {glm::vec3{h, h, h}, glm::vec2{1, 1}},
        {glm::vec3{-h, h, h}, glm::vec2{0, 1}},
        // Back (Z−)
        {glm::vec3{h, -h, -h}, glm::vec2{0, 0}},
        {glm::vec3{-h, -h, -h}, glm::vec2{1, 0}},
        {glm::vec3{-h, h, -h}, glm::vec2{1, 1}},
        {glm::vec3{h, h, -h}, glm::vec2{0, 1}},
        // Left (X−)
        {glm::vec3{-h, -h, -h}, glm::vec2{0, 0}},
        {glm::vec3{-h, -h, h}, glm::vec2{1, 0}},
        {glm::vec3{-h, h, h}, glm::vec2{1, 1}},
        {glm::vec3{-h, h, -h}, glm::vec2{0, 1}},
        // Right (X+)
        {glm::vec3{h, -h, h}, glm::vec2{0, 0}},
        {glm::vec3{h, -h, -h}, glm::vec2{1, 0}},
        {glm::vec3{h, h, -h}, glm::vec2{1, 1}},
        {glm::vec3{h, h, h}, glm::vec2{0, 1}},
        // Top (Y+)
        {glm::vec3{-h, h, h}, glm::vec2{0, 0}},
        {glm::vec3{h, h, h}, glm::vec2{1, 0}},
        {glm::vec3{h, h, -h}, glm::vec2{1, 1}},
        {glm::vec3{-h, h, -h}, glm::vec2{0, 1}},
        // Bottom (Y−)
        {glm::vec3{-h, -h, -h}, glm::vec2{0, 0}},
        {glm::vec3{h, -h, -h}, glm::vec2{1, 0}},
        {glm::vec3{h, -h, h}, glm::vec2{1, 1}},
        {glm::vec3{-h, -h, h}, glm::vec2{0, 1}},
    };
}

/**
 * @brief Returns indices of a cube.
 * @return Indices of the cube.
 */
static std::vector<uint16_t> CreateCubeIndices()
{
    return {
        0,  1,  2,  0,  2,  3,  // Front
        4,  5,  6,  4,  6,  7,  // Back
        8,  9,  10, 8,  10, 11, // Left
        12, 13, 14, 12, 14, 15, // Right
        16, 17, 18, 16, 18, 19, // Top
        20, 21, 22, 20, 22, 23  // Bottom
    };
}

/**
 * @brief Calculates and returns vertices of a quad which placed in XY plane.
 * @tparam VertexType Structure of a vertex.
 * @param size Size of the quad.
 * @return Vertices of the aud (XY).
 */
template<typename VertexType>
std::vector<VertexType> CreateQuadVerticesXY(float size);

template<>
inline std::vector<VertexPos3Uv2> CreateQuadVerticesXY(const float size)
{
    return {
        {glm::vec3{-size, -size, 0.0f}, glm::vec2{0.0f, 0.0f}}, // 0
        {glm::vec3{size, -size, 0.0f}, glm::vec2{1.0f, 0.0f}},  // 1
        {glm::vec3{size, size, 0.0f}, glm::vec2{1.0f, 1.0f}},   // 2
        {glm::vec3{-size, size, 0.0f}, glm::vec2{0.0f, 1.0f}},  // 3
    };
}

/**
 * @brief Calculates and returns vertices of a quad which placed in XZ plane.
 * @tparam VertexType Structure of a vertex.
 * @param size Size of the quad.
 * @return Vertices of the aud (XZ).
 */
template<typename VertexType>
std::vector<VertexType> CreateQuadVerticesXZ(float size);

template<>
inline std::vector<VertexPos3Uv2> CreateQuadVerticesXZ(const float size)
{
    return {
        {glm::vec3{-size, 0.0f, -size}, glm::vec2{0.0f, 0.0f}}, // 0
        {glm::vec3{size, 0.0f, -size}, glm::vec2{1.0f, 0.0f}},  // 1
        {glm::vec3{size, 0.0f, size}, glm::vec2{1.0f, 1.0f}},   // 2
        {glm::vec3{-size, 0.0f, size}, glm::vec2{0.0f, 1.0f}},  // 3
    };
}

/**
 * @brief Returns indices of a quad.
 * @return Indices of the quad.
 */
static std::vector<uint16_t> CreateQuadIndices()
{
    return {
        0, 1, 2, // First triangle
        2, 3, 0  // Second triangle
    };
}

} // namespace common::utility
