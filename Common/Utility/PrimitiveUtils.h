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

#include <array>
#include <numbers>
#include <vector>

namespace common::utility
{

template<typename VertexType>
std::vector<VertexType> CreateSphereVertices(const float radius, const uint32_t sectorCount, const uint32_t stackCount)
{
    std::vector<VertexType> vertices;

    for (uint32_t i = 0; i <= stackCount; ++i) {
        const auto stackAngle = static_cast<float>(std::numbers::pi / 2.0 - i * std::numbers::pi / stackCount);
        const float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (uint32_t j = 0; j <= sectorCount; ++j) {
            const float sectorAngle = j * 2 * std::numbers::pi / sectorCount;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            float u = static_cast<float>(j) / static_cast<float>(sectorCount);
            float v = static_cast<float>(i) / static_cast<float>(stackCount);

            vertices.push_back(VertexType{{x, y, z}, {u, v}});
        }
    }

    return vertices;
}

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

template<typename VertexType>
std::vector<VertexType> CreateCubeVertices(const float size)
{
    const float h = size * 0.5f;

    return {
        // Front (Z+)
        {{-h, -h, h}, {0, 0}},
        {{h, -h, h}, {1, 0}},
        {{h, h, h}, {1, 1}},
        {{-h, h, h}, {0, 1}},
        // Back (Z−)
        {{h, -h, -h}, {0, 0}},
        {{-h, -h, -h}, {1, 0}},
        {{-h, h, -h}, {1, 1}},
        {{h, h, -h}, {0, 1}},
        // Left (X−)
        {{-h, -h, -h}, {0, 0}},
        {{-h, -h, h}, {1, 0}},
        {{-h, h, h}, {1, 1}},
        {{-h, h, -h}, {0, 1}},
        // Right (X+)
        {{h, -h, h}, {0, 0}},
        {{h, -h, -h}, {1, 0}},
        {{h, h, -h}, {1, 1}},
        {{h, h, h}, {0, 1}},
        // Top (Y+)
        {{-h, h, h}, {0, 0}},
        {{h, h, h}, {1, 0}},
        {{h, h, -h}, {1, 1}},
        {{-h, h, -h}, {0, 1}},
        // Bottom (Y−)
        {{-h, -h, -h}, {0, 0}},
        {{h, -h, -h}, {1, 0}},
        {{h, -h, h}, {1, 1}},
        {{-h, -h, h}, {0, 1}},
    };
}

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

} // namespace common::utility
