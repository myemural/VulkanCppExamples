/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ScenePrimitives.h"

#include <cmath>
#include <numbers>

#include <glm/detail/func_geometric.inl>

namespace common::vulkan_framework
{

std::vector<glm::vec3> CreateCubePositions(const float size)
{
    float h = size * 0.5f;

    return {
        // Front (+Z)
        {-h, -h, h},
        {h, -h, h},
        {h, h, h},
        {-h, h, h},
        // Back (-Z)
        {h, -h, -h},
        {-h, -h, -h},
        {-h, h, -h},
        {h, h, -h},
        // Left (-X)
        {-h, -h, -h},
        {-h, -h, h},
        {-h, h, h},
        {-h, h, -h},
        // Right (+X)
        {h, -h, h},
        {h, -h, -h},
        {h, h, -h},
        {h, h, h},
        // Top (+Y)
        {-h, h, h},
        {h, h, h},
        {h, h, -h},
        {-h, h, -h},
        // Bottom (-Y)
        {-h, -h, h},
        {h, -h, h},
        {h, -h, -h},
        {-h, -h, -h},
    };
}

std::vector<glm::vec2> CreateCubeUVs()
{
    return {// Front (+Z)
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1},
            // Back (-Z)
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1},
            // Left (-X)
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1},
            // Right (+X)
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1},
            // Top (+Y)
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1},
            // Bottom (-Y)
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1}};
}

std::vector<glm::vec3> CreateCubeNormals()
{
    return {
        // Front (+Z)
        {0, 0, 1},
        {0, 0, 1},
        {0, 0, 1},
        {0, 0, 1},
        // Back (-Z)
        {0, 0, -1},
        {0, 0, -1},
        {0, 0, -1},
        {0, 0, -1},
        // Left (-X)
        {-1, 0, 0},
        {-1, 0, 0},
        {-1, 0, 0},
        {-1, 0, 0},
        // Right (+X)
        {1, 0, 0},
        {1, 0, 0},
        {1, 0, 0},
        {1, 0, 0},
        // Top (+Y)
        {0, 1, 0},
        {0, 1, 0},
        {0, 1, 0},
        {0, 1, 0},
        // Bottom (-Y)
        {0, -1, 0},
        {0, -1, 0},
        {0, -1, 0},
        {0, -1, 0},
    };
}

std::vector<glm::vec4> CreateCubeTangents()
{
    return {
        // Front (+Z)
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        // Back (-Z)
        {-1, 0, 0, 1},
        {-1, 0, 0, 1},
        {-1, 0, 0, 1},
        {-1, 0, 0, 1},
        // Left (-X)
        {0, 0, 1, 1},
        {0, 0, 1, 1},
        {0, 0, 1, 1},
        {0, 0, 1, 1},
        // Right (+X)
        {0, 0, -1, 1},
        {0, 0, -1, 1},
        {0, 0, -1, 1},
        {0, 0, -1, 1},
        // Top (+Y)
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        {1, 0, 0, 1},
        // Bottom (-Y)
        {1, 0, 0, -1},
        {1, 0, 0, -1},
        {1, 0, 0, -1},
        {1, 0, 0, -1},
    };
}

std::vector<std::uint16_t> CreateCubeIndices()
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

std::vector<glm::vec3>
CreateSpherePositions(const float size, const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec3> positions;
    positions.reserve((stackCount + 1) * (sectorCount + 1));

    const float radius = size * 0.5f;
    constexpr float pi = std::numbers::pi_v<float>;

    for (int i = 0; i <= stackCount; ++i) {
        const float stackAngle = pi * 0.5f - static_cast<float>(i) * (pi / static_cast<float>(stackCount));
        const float xy = radius * std::cos(stackAngle);
        const float z = radius * std::sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            const float sectorAngle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));
            const float x = xy * std::cos(sectorAngle);
            const float y = xy * std::sin(sectorAngle);

            positions.emplace_back(x, y, z);
        }
    }

    return positions;
}

std::vector<glm::vec2> CreateSphereUVs(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec2> uvs;
    uvs.reserve((stackCount + 1) * (sectorCount + 1));

    for (int i = 0; i <= stackCount; ++i) {
        float v = static_cast<float>(i) / static_cast<float>(stackCount);

        for (int j = 0; j <= sectorCount; ++j) {
            float u = static_cast<float>(j) / static_cast<float>(sectorCount);
            uvs.emplace_back(u, v);
        }
    }

    return uvs;
}

std::vector<glm::vec3> CreateSphereNormals(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec3> normals;
    normals.reserve((stackCount + 1) * (sectorCount + 1));

    constexpr float pi = std::numbers::pi_v<float>;

    for (int i = 0; i <= stackCount; ++i) {
        const float stackAngle = pi * 0.5f - static_cast<float>(i) * (pi / static_cast<float>(stackCount));
        const float xy = std::cos(stackAngle);
        const float z = std::sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            const float sectorAngle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));
            const float x = xy * std::cos(sectorAngle);
            const float y = xy * std::sin(sectorAngle);

            normals.emplace_back(x, y, z);
        }
    }

    return normals;
}

std::vector<glm::vec4> CreateSphereTangents(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec4> tangents;
    tangents.reserve((stackCount + 1) * (sectorCount + 1));

    constexpr float pi = std::numbers::pi_v<float>;

    for (int i = 0; i <= stackCount; ++i) {
        const float stackAngle = pi * 0.5f - static_cast<float>(i) * (pi / static_cast<float>(stackCount));
        const float xy = std::cos(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            const float sectorAngle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));

            glm::vec3 tangent(-xy * std::sin(sectorAngle), xy * std::cos(sectorAngle), 0.0f);

            tangents.emplace_back(glm::normalize(tangent), 1.0f);
        }
    }

    return tangents;
}

std::vector<std::uint16_t> CreateSphereIndices(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<std::uint16_t> indices;
    indices.reserve(stackCount * sectorCount * 6);

    for (int i = 0; i < stackCount; ++i) {
        std::uint32_t k1 = i * (sectorCount + 1);
        std::uint32_t k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // Upper triangle
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1);

            // Lower triangle
            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);
        }
    }

    return indices;
}

std::vector<glm::vec3>
CreateConePositions(const float height, const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec3> positions;
    positions.reserve((stackCount + 1) * (sectorCount + 1) + (sectorCount + 2));

    const float radius = height * 0.5f;
    const float baseY = -height * 0.5f;
    constexpr float pi = std::numbers::pi_v<float>;

    // Side
    for (int i = 0; i <= stackCount; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(stackCount);
        const float y = baseY + t * height;
        const float r = radius * (1.0f - t);

        for (int j = 0; j <= sectorCount; ++j) {
            const float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));
            const float x = r * std::cos(angle);
            const float z = r * std::sin(angle);

            positions.emplace_back(x, y, z);
        }
    }

    // Base center
    positions.emplace_back(0.0f, baseY, 0.0f);

    // Base ring
    for (int j = 0; j <= sectorCount; ++j) {
        const float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));
        positions.emplace_back(radius * cosf(angle), baseY, radius * sinf(angle));
    }

    return positions;
}

std::vector<glm::vec2> CreateConeUVs(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec2> uvs;
    uvs.reserve((stackCount + 1) * (sectorCount + 1) + (sectorCount + 2));

    constexpr float pi = std::numbers::pi_v<float>;

    // Side UV
    for (int i = 0; i <= stackCount; ++i) {
        const float v = static_cast<float>(i) / static_cast<float>(stackCount);
        for (int j = 0; j <= sectorCount; ++j) {
            const float u = static_cast<float>(j) / static_cast<float>(sectorCount);
            uvs.emplace_back(u, v);
        }
    }

    // Base center UV
    uvs.emplace_back(0.5f, 0.5f);

    // Base ring UV
    for (int j = 0; j <= sectorCount; ++j) {
        const float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));
        const float x = std::cos(angle) * 0.5f;
        const float y = std::sin(angle) * 0.5f;
        uvs.emplace_back(0.5f + x, 0.5f + y);
    }

    return uvs;
}

std::vector<glm::vec3> CreateConeNormals(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec3> normals;
    normals.reserve((stackCount + 1) * (sectorCount + 1) + (sectorCount + 2));

    constexpr float pi = std::numbers::pi_v<float>;

    // Side normals
    for (int i = 0; i <= stackCount; ++i) {
        for (int j = 0; j <= sectorCount; ++j) {

            const float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));
            const float x = std::cos(angle);
            const float z = std::sin(angle);

            constexpr float slope = 0.5f; // Slope is constant, because always radius = height * 0.5
            glm::vec3 n(x, slope, z);
            n = glm::normalize(n);

            normals.emplace_back(n);
        }
    }

    // Base center normals
    normals.emplace_back(0.0f, -1.0f, 0.0f);

    // Base ring normals
    for (int j = 0; j <= sectorCount; ++j) {
        normals.emplace_back(0.0f, -1.0f, 0.0f);
    }

    return normals;
}

std::vector<glm::vec4> CreateConeTangents(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec4> tangents;
    tangents.reserve((stackCount + 1) * (sectorCount + 1) + (sectorCount + 2));

    constexpr float pi = std::numbers::pi_v<float>;

    // Side
    for (int i = 0; i <= stackCount; ++i) {
        for (int j = 0; j <= sectorCount; ++j) {
            const float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));

            glm::vec3 tangent(-std::sin(angle), 0.0f, std::cos(angle));
            tangents.emplace_back(glm::normalize(tangent), 1.0f);
        }
    }

    // Base center
    tangents.emplace_back(1.0f, 0.0f, 0.0f, -1.0f);

    // Base ring
    for (int j = 0; j <= sectorCount; ++j) {
        tangents.emplace_back(1.0f, 0.0f, 0.0f, -1.0f);
    }

    return tangents;
}

std::vector<uint16_t> CreateConeIndices(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<uint16_t> indices;
    indices.reserve(stackCount * sectorCount * 6 + sectorCount * 3);

    const std::uint32_t ring = sectorCount + 1;

    // Side indices
    for (int i = 0; i < stackCount; ++i) {
        const std::uint32_t k1 = i * ring;  // Current ring
        const std::uint32_t k2 = k1 + ring; // Next ring

        for (int j = 0; j < sectorCount; ++j) {

            // First triangle
            indices.push_back(k1 + j);
            indices.push_back(k2 + j);
            indices.push_back(k1 + j + 1);

            // Second triangle
            indices.push_back(k1 + j + 1);
            indices.push_back(k2 + j);
            indices.push_back(k2 + j + 1);
        }
    }

    // Base indices
    const std::uint32_t baseCenter = (stackCount + 1) * ring;
    const std::uint32_t baseStart = baseCenter + 1;

    // Base ring indices
    for (int j = 0; j < sectorCount; ++j) {
        indices.push_back(baseCenter);
        indices.push_back(baseStart + j);
        indices.push_back(baseStart + j + 1);
    }

    return indices;
}

std::vector<glm::vec3>
CreateCylinderPositions(const float height, const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec3> positions;
    positions.reserve((stackCount + 1) * (sectorCount + 1) + 2 * (sectorCount + 1));

    const float radius = height * 0.5f;
    const float halfH = height * 0.5f;
    constexpr float pi = std::numbers::pi_v<float>;

    // Side surface
    for (int i = 0; i <= stackCount; ++i) {
        const float y = halfH - (height * static_cast<float>(i) / static_cast<float>(stackCount));

        for (int j = 0; j <= sectorCount; ++j) {
            const float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));
            positions.emplace_back(radius * std::cos(angle), y, radius * std::sin(angle));
        }
    }

    // Top disk
    for (int j = 0; j <= sectorCount; ++j) {
        const float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));
        positions.emplace_back(radius * std::cos(angle), halfH, radius * std::sin(angle));
    }

    // Bottom disk
    for (int j = 0; j <= sectorCount; ++j) {
        const float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));
        positions.emplace_back(radius * std::cos(angle), -halfH, radius * std::sin(angle));
    }

    return positions;
}

std::vector<glm::vec2> CreateCylinderUVs(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec2> uvs;
    uvs.reserve((stackCount + 1) * (sectorCount + 1) + 2 * (sectorCount + 1));

    constexpr float pi = std::numbers::pi_v<float>;

    // Side surface
    for (int i = 0; i <= stackCount; ++i) {
        float v = static_cast<float>(i) / static_cast<float>(stackCount);

        for (int j = 0; j <= sectorCount; ++j) {
            float u = static_cast<float>(j) / static_cast<float>(sectorCount);
            uvs.emplace_back(u, v);
        }
    }

    auto makeDiskUV = [&](const int j) {
        const float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));
        const float u = 0.5f + 0.5f * std::cos(angle);
        const float v = 0.5f + 0.5f * std::sin(angle);
        return glm::vec2(u, v);
    };

    // Top disk
    for (int j = 0; j <= sectorCount; ++j) {
        uvs.emplace_back(makeDiskUV(j));
    }

    // Bottom disk
    for (int j = 0; j <= sectorCount; ++j) {
        uvs.emplace_back(makeDiskUV(j));
    }

    return uvs;
}

std::vector<glm::vec3> CreateCylinderNormals(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec3> normals;
    normals.reserve((stackCount + 1) * (sectorCount + 1) + 2 * (sectorCount + 1));

    constexpr float pi = std::numbers::pi_v<float>;

    // Side surface
    for (int i = 0; i <= stackCount; ++i) {
        for (int j = 0; j <= sectorCount; ++j) {
            const float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));
            normals.emplace_back(std::cos(angle), 0.0f, std::sin(angle));
        }
    }

    // Top disk
    for (int j = 0; j <= sectorCount; ++j) {
        normals.emplace_back(0.0f, 1.0f, 0.0f);
    }

    // Bottom disk
    for (int j = 0; j <= sectorCount; ++j) {
        normals.emplace_back(0.0f, -1.0f, 0.0f);
    }

    return normals;
}

std::vector<glm::vec4> CreateCylinderTangents(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<glm::vec4> tangents;
    tangents.reserve((stackCount + 1) * (sectorCount + 1) + 2 * (sectorCount + 1));

    constexpr float pi = std::numbers::pi_v<float>;

    // Side surface
    for (int i = 0; i <= stackCount; ++i) {
        for (int j = 0; j <= sectorCount; ++j) {
            const float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectorCount));

            glm::vec3 tangent(-std::sin(angle), 0.0f, std::cos(angle));
            tangents.emplace_back(glm::normalize(tangent), 1.0f);
        }
    }

    // Top disk
    for (int j = 0; j <= sectorCount; ++j) {
        tangents.emplace_back(1.0f, 0.0f, 0.0f, 1.0f);
    }

    // Bottom disk
    for (int j = 0; j <= sectorCount; ++j) {
        tangents.emplace_back(1.0f, 0.0f, 0.0f, -1.0f);
    }

    return tangents;
}

std::vector<uint16_t> CreateCylinderIndices(const std::uint32_t stackCount, const std::uint32_t sectorCount)
{
    std::vector<uint16_t> indices;
    const std::uint32_t ring = sectorCount + 1;

    // Side surface
    for (int i = 0; i < stackCount; ++i) {
        const std::uint32_t k1 = i * ring;
        const std::uint32_t k2 = k1 + ring;

        for (int j = 0; j < sectorCount; ++j) {
            indices.push_back(k1 + j);
            indices.push_back(k2 + j);
            indices.push_back(k1 + j + 1);

            indices.push_back(k1 + j + 1);
            indices.push_back(k2 + j);
            indices.push_back(k2 + j + 1);
        }
    }

    // Top disk
    const std::uint32_t topStart = (stackCount + 1) * ring;
    const std::uint32_t topCenter = topStart + sectorCount;

    for (int j = 0; j < sectorCount; ++j) {
        indices.push_back(topCenter);
        indices.push_back(topStart + j);
        indices.push_back(topStart + j + 1);
    }

    // Bottom disk
    const std::uint32_t bottomStart = topStart + ring;
    const std::uint32_t bottomCenter = bottomStart + sectorCount;

    for (int j = 0; j < sectorCount; ++j) {
        indices.push_back(bottomCenter);
        indices.push_back(bottomStart + j + 1);
        indices.push_back(bottomStart + j);
    }

    return indices;
}

std::vector<glm::vec3> CreatePlanePositions(const float size)
{
    std::vector<glm::vec3> positions;
    positions.reserve(4);

    const float half = size * 0.5f;

    positions.emplace_back(-half, 0.0f, -half);
    positions.emplace_back(half, 0.0f, -half);
    positions.emplace_back(half, 0.0f, half);
    positions.emplace_back(-half, 0.0f, half);

    return positions;
}

std::vector<glm::vec2> CreatePlaneUVs()
{
    std::vector<glm::vec2> uvs;
    uvs.reserve(4);

    uvs.emplace_back(0.0f, 0.0f);
    uvs.emplace_back(1.0f, 0.0f);
    uvs.emplace_back(1.0f, 1.0f);
    uvs.emplace_back(0.0f, 1.0f);

    return uvs;
}

std::vector<glm::vec3> CreatePlaneNormals()
{
    std::vector<glm::vec3> normals;
    normals.reserve(4);

    normals.emplace_back(0.0f, 1.0f, 0.0f);
    normals.emplace_back(0.0f, 1.0f, 0.0f);
    normals.emplace_back(0.0f, 1.0f, 0.0f);
    normals.emplace_back(0.0f, 1.0f, 0.0f);

    return normals;
}

std::vector<glm::vec4> CreatePlaneTangents()
{
    return {
        {1.0f, 0.0f, 0.0f, -1.0f},
        {1.0f, 0.0f, 0.0f, -1.0f},
        {1.0f, 0.0f, 0.0f, -1.0f},
        {1.0f, 0.0f, 0.0f, -1.0f},
    };
}

std::vector<std::uint16_t> CreatePlaneIndices() { return {0, 1, 2, 2, 3, 0}; }

} // namespace common::vulkan_framework
