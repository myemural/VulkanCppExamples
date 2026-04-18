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

#include <cmath>
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
std::vector<VertexType> CreateSphereVertices(float radius, std::uint32_t sectorCount, std::uint32_t stackCount);

template<>
inline std::vector<VertexPos3Uv2>
CreateSphereVertices(const float radius, const std::uint32_t sectorCount, const std::uint32_t stackCount)
{
    std::vector<VertexPos3Uv2> vertices;

    for (auto i = 0U; i <= stackCount; ++i) {
        const auto stackAngle = static_cast<float>(std::numbers::pi / 2.0 - i * std::numbers::pi / stackCount);
        const float xy = radius * std::cos(stackAngle);
        const float z = radius * std::sin(stackAngle);

        for (auto j = 0U; j <= sectorCount; ++j) {
            const auto sectorAngle = static_cast<float>(j * 2 * std::numbers::pi / sectorCount);

            const float x = xy * std::cos(sectorAngle);
            const float y = xy * std::sin(sectorAngle);

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
static std::vector<std::uint16_t> CreateSphereIndices(const std::uint32_t sectorCount, const std::uint32_t stackCount)
{
    std::vector<std::uint16_t> indices;

    for (auto i = 0U; i < stackCount; ++i) {
        std::uint32_t k1 = i * (sectorCount + 1);
        std::uint32_t k2 = k1 + sectorCount + 1;

        for (auto j = 0U; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != stackCount - 1) {
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
static std::vector<std::uint16_t> CreateCubeIndices()
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
static std::vector<std::uint16_t> CreateQuadIndices()
{
    return {
        0, 1, 2, // First triangle
        2, 3, 0  // Second triangle
    };
}

/**
 * @brief Calculates and returns vertices of a cone.
 * @tparam VertexType Structure of a vertex.
 * @param radius Base radius of the cone.
 * @param height Height of the cone.
 * @param sectorCount Sector count of the cone.
 * @param stackCount Stack count of the cone.
 * @return Vertices for cone.
 */
template<typename VertexType>
std::vector<VertexType>
CreateConeVertices(float radius, float height, std::uint32_t sectorCount, std::uint32_t stackCount);

template<>
inline std::vector<VertexPos3Uv2> CreateConeVertices(const float radius,
                                                     const float height,
                                                     const std::uint32_t sectorCount,
                                                     const std::uint32_t stackCount)
{
    std::vector<VertexPos3Uv2> vertices;
    vertices.reserve((stackCount + 1) * (sectorCount + 1) + (sectorCount + 2));

    // Side surface
    for (auto i = 0U; i <= stackCount; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(stackCount); // 0 for base point, 1 for peak point
        const float r = radius * (1.0f - t);                                    // The radius gradually decreases
        const float z = height * t;                                             // The height gradually increases

        for (auto j = 0U; j <= sectorCount; ++j) {
            const auto sectorAngle = static_cast<float>(j) * 2.0f * static_cast<float>(std::numbers::pi / sectorCount);

            const float x = r * std::cos(sectorAngle);
            const float y = r * std::sin(sectorAngle);

            const float u = static_cast<float>(j) / static_cast<float>(sectorCount);
            const float v = t;

            vertices.push_back({glm::vec3{x, y, z}, glm::vec2{u, v}});
        }
    }

    // Base center point
    vertices.push_back({glm::vec3{0, 0, 0}, glm::vec2{0.5f, 0.5f}});

    // Base circumference
    for (auto j = 0U; j <= sectorCount; ++j) {
        const float angle = static_cast<float>(j) * 2.0f * static_cast<float>(std::numbers::pi / sectorCount);
        const float x = radius * std::cos(angle);
        const float y = radius * std::sin(angle);
        vertices.push_back({glm::vec3{x, y, 0}, glm::vec2{(std::cos(angle) + 1.0f) * 0.5f, (sinf(angle) + 1.0f) * 0.5f}});
    }

    return vertices;
}

/**
 * @brief Calculates and returns indices of a cone.
 * @param sectorCount Sector count of the cone.
 * @param stackCount Stack count of the cone.
 * @return Indices of the cone.
 */
inline std::vector<std::uint16_t> CreateConeIndices(const std::uint32_t sectorCount, const std::uint32_t stackCount)
{
    std::vector<std::uint16_t> indices;

    const std::uint32_t ringVertexCount = sectorCount + 1;

    // Side surface triangles
    for (auto i = 0U; i < stackCount; ++i) {
        const std::uint32_t currentRow = i * ringVertexCount;
        const std::uint32_t nextRow = (i + 1) * ringVertexCount;

        for (auto j = 0U; j < sectorCount; ++j) {
            std::uint16_t i0 = currentRow + j;
            std::uint16_t i1 = nextRow + j;
            std::uint16_t i2 = nextRow + j + 1;
            std::uint16_t i3 = currentRow + j + 1;

            // Create 1 quad from 2 triangles
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    // Base triangles
    const std::uint16_t baseCenter = stackCount * ringVertexCount + ringVertexCount;
    const std::uint16_t baseStart = baseCenter + 1;

    for (auto j = 0U; j < sectorCount; ++j) {
        std::uint16_t i0 = baseCenter;
        std::uint16_t i1 = baseStart + j;
        std::uint16_t i2 = baseStart + j + 1;
        indices.push_back(i0);
        indices.push_back(i2);
        indices.push_back(i1);
    }

    return indices;
}

/**
 * @brief Calculates and returns vertices of a cylinder.
 * @tparam VertexType Structure of a vertex.
 * @param radius Radius of the cylinder.
 * @param height Height of the cylinder.
 * @param sectorCount Sector count of the cylinder.
 * @param stackCount Stack count of the cylinder.
 * @return Vertices for cylinder.
 */
template<typename VertexType>
std::vector<VertexType>
CreateCylinderVertices(float radius, float height, std::uint32_t sectorCount, std::uint32_t stackCount);

template<>
inline std::vector<VertexPos3Uv2> CreateCylinderVertices(const float radius,
                                                         const float height,
                                                         const std::uint32_t sectorCount,
                                                         const std::uint32_t stackCount)
{
    std::vector<VertexPos3Uv2> vertices;
    vertices.reserve((stackCount + 1) * (sectorCount + 1) + (sectorCount + 1) * 2);

    const float halfH = height * 0.5f;

    // Side surface (body)
    for (auto i = 0U; i <= stackCount; ++i) {
        const float h = halfH - static_cast<float>(i) / static_cast<float>(stackCount) * height; // From top to bottom

        for (auto j = 0U; j <= sectorCount; ++j) {
            const float angle = static_cast<float>(j) * 2.0f * static_cast<float>(std::numbers::pi / sectorCount);

            const float x = radius * std::cos(angle);
            const float y = radius * std::sin(angle);
            const float z = h;

            const float u = static_cast<float>(j) / static_cast<float>(sectorCount);
            const float v = static_cast<float>(i) / static_cast<float>(stackCount);

            vertices.push_back({glm::vec3{x, y, z}, glm::vec2{u, v}});
        }
    }

    // Top cap rim
    for (auto j = 0U; j <= sectorCount; ++j) {
        const float angle = static_cast<float>(j) * 2.0f * static_cast<float>(std::numbers::pi / sectorCount);

        const float x = radius * std::cos(angle);
        const float y = radius * std::sin(angle);
        const float z = +halfH;

        const float u = (std::cos(angle) + 1.0f) * 0.5f;
        const float v = (std::sin(angle) + 1.0f) * 0.5f;

        vertices.push_back({glm::vec3{x, y, z}, glm::vec2{u, v}});
    }

    // Top cap center
    vertices.push_back({glm::vec3{0, 0, +halfH}, glm::vec2{0.5f, 0.5f}});

    // Bottom cap rim
    for (auto j = 0U; j <= sectorCount; ++j) {
        const float angle = static_cast<float>(j) * 2.0f * static_cast<float>(std::numbers::pi / sectorCount);

        const float x = radius * std::cos(angle);
        const float y = radius * std::sin(angle);
        const float z = -halfH;

        const float u = (std::cos(angle) + 1.0f) * 0.5f;
        const float v = (std::sin(angle) + 1.0f) * 0.5f;

        vertices.push_back({glm::vec3{x, y, z}, glm::vec2{u, v}});
    }

    // Bottom cap center
    vertices.push_back({glm::vec3{0, 0, -halfH}, glm::vec2{0.5f, 0.5f}});

    return vertices;
}

/**
 * @brief Calculates and returns indices of a cylinder.
 * @param sectorCount Sector count of the cylinder.
 * @param stackCount Stack count of the cylinder.
 * @return Indices of the cylinder.
 */
inline std::vector<std::uint16_t> CreateCylinderIndices(const std::uint32_t sectorCount, const std::uint32_t stackCount)
{
    std::vector<std::uint16_t> indices;

    const std::uint32_t ringVertices = sectorCount + 1;

    // Body indices
    for (auto i = 0U; i < stackCount; ++i) {
        const std::uint16_t k1 = i * ringVertices;
        const std::uint16_t k2 = (i + 1) * ringVertices;

        for (auto j = 0U; j < sectorCount; ++j) {
            std::uint16_t a = k1 + j;
            std::uint16_t b = k2 + j;
            std::uint16_t c = k1 + j + 1;
            std::uint16_t d = k2 + j + 1;

            // Create 1 quad from 2 triangles
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(c);

            indices.push_back(c);
            indices.push_back(b);
            indices.push_back(d);
        }
    }

    // Top cap indices
    const std::uint16_t topCenter = stackCount * ringVertices + ringVertices;
    const std::uint16_t topStart = topCenter + 1;

    for (auto j = 0U; j < sectorCount; ++j) {
        indices.push_back(topCenter);
        indices.push_back(topStart + j);
        indices.push_back(topStart + j + 1);
    }

    // Bottom cap indices
    const std::uint16_t bottomCenter = topStart + (sectorCount + 1);
    const std::uint16_t bottomStart = bottomCenter + 1;

    for (auto j = 0U; j < sectorCount; ++j) {
        indices.push_back(bottomCenter);
        indices.push_back(bottomStart + j + 1);
        indices.push_back(bottomStart + j);
    }

    return indices;
}

} // namespace common::utility
