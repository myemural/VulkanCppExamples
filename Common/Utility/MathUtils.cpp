/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "MathUtils.h"

#include <optional>

namespace common::utility
{

std::uint32_t CeilDiv(const std::uint32_t value, const std::uint32_t divisor)
{
    return (value + (divisor - 1)) / divisor;
}

std::vector<glm::vec3> GenerateRandomPositions(const size_t count,
                                               const glm::vec3 minBounds,
                                               const glm::vec3 maxBounds,
                                               const float minDistance)
{
    std::vector<glm::vec3> positions;
    positions.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution distX(minBounds.x, maxBounds.x);
    std::uniform_real_distribution distY(minBounds.y, maxBounds.y);
    std::uniform_real_distribution distZ(minBounds.z, maxBounds.z);

    for (size_t i = 0; i < count; ++i) {
        glm::vec3 pos;
        bool valid;
        int currentAttemptCount = 0;

        do {
            pos = glm::vec3(distX(gen), distY(gen), distZ(gen));
            valid = true;
            for (const auto& existingPos: positions) {
                if (glm::distance(pos, existingPos) < minDistance) {
                    valid = false;
                    break;
                }
            }

            // To avoid infinite loops we should add attempt count and max number of attempts
            if (constexpr int maxNumberOfAttempts = 1000; ++currentAttemptCount > maxNumberOfAttempts) {
                break;
            }
        } while (!valid);
        positions.push_back(pos);
    }

    return positions;
}

std::vector<glm::vec3> GenerateRandomPositionsOnBounds(const size_t count,
                                                       const glm::vec3 minBounds,
                                                       const glm::vec3 maxBounds,
                                                       const float minDistance)
{
    std::vector<glm::vec3> positions;
    positions.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());

    const float xMin = minBounds.x, xMax = maxBounds.x;
    const float yMin = minBounds.y, yMax = maxBounds.y;
    const float zMin = minBounds.z, zMax = maxBounds.z;

    // std::nullopt means random distribution position, fixed values means fixed position
    struct Region
    {
        std::optional<float> x;
        std::optional<float> y;
        std::optional<float> z;
    };

    const std::vector<Region> regions = {
        // Faces
        {xMin, std::nullopt, std::nullopt}, // Left face
        {xMax, std::nullopt, std::nullopt}, // Right face
        {std::nullopt, yMin, std::nullopt}, // Bottom face
        {std::nullopt, yMax, std::nullopt}, // Top face
        {std::nullopt, std::nullopt, zMin}, // Front face
        {std::nullopt, std::nullopt, zMax}, // Back face

        // Edges
        {std::nullopt, yMin, zMin}, // Bottom-front
        {std::nullopt, yMax, zMin}, // Top-front
        {std::nullopt, yMin, zMax}, // Bottom-back
        {std::nullopt, yMax, zMax}, // Top-back
        {xMin, std::nullopt, zMin}, // Left-front
        {xMax, std::nullopt, zMin}, // Right-front
        {xMin, std::nullopt, zMax}, // Left-back
        {xMax, std::nullopt, zMax}, // Right-back
        {xMin, yMin, std::nullopt}, // Left-bottom
        {xMax, yMin, std::nullopt}, // Right-bottom
        {xMin, yMax, std::nullopt}, // Left-top
        {xMax, yMax, std::nullopt}, // Right-top

        // Corners
        {xMin, yMin, zMin},
        {xMax, yMin, zMin},
        {xMin, yMax, zMin},
        {xMax, yMax, zMin},
        {xMin, yMin, zMax},
        {xMax, yMin, zMax},
        {xMin, yMax, zMax},
        {xMax, yMax, zMax},
    };

    std::uniform_int_distribution<size_t> regionDist(0, regions.size() - 1);
    std::uniform_real_distribution<float> distX(xMin, xMax);
    std::uniform_real_distribution<float> distY(yMin, yMax);
    std::uniform_real_distribution<float> distZ(zMin, zMax);

    for (size_t i = 0; i < count; ++i) {
        glm::vec3 pos;
        bool valid = false;
        int currentAttemptCount = 0;
        constexpr int maxNumberOfAttempts = 1000;

        do {
            const auto& [x, y, z] = regions[regionDist(gen)];

            pos = glm::vec3(x.value_or(distX(gen)), y.value_or(distY(gen)), z.value_or(distZ(gen)));
            valid = true;

            for (const auto& existingPos: positions) {
                if (glm::distance(pos, existingPos) < minDistance) {
                    valid = false;
                    break;
                }
            }

        } while (!valid && ++currentAttemptCount < maxNumberOfAttempts);

        positions.push_back(pos);
    }

    return positions;
}

glm::vec3 GenerateRandomColor(const float minValue, const float maxValue)
{
    std::mt19937 generator{std::random_device{}()};
    std::uniform_real_distribution distribution(minValue, maxValue);

    return glm::vec3{distribution(generator), distribution(generator), distribution(generator)};
}

std::uint32_t GetMipLevelCount(const std::uint32_t textureWidth, const std::uint32_t textureHeight)
{
    return static_cast<uint32_t>(std::floor(std::log2(std::max(textureWidth, textureHeight)))) + 1;
}

glm::mat4 MakeCubemapView(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up)
{
    const glm::vec3 f = glm::normalize(dir);
    const glm::vec3 r = glm::normalize(glm::cross(f, up));
    const glm::vec3 u = glm::cross(r, f);

    glm::mat4 m(1.0f);
    m[0][0] = r.x;
    m[1][0] = r.y;
    m[2][0] = r.z;
    m[0][1] = u.x;
    m[1][1] = u.y;
    m[2][1] = u.z;
    m[0][2] = -f.x;
    m[1][2] = -f.y;
    m[2][2] = -f.z;
    m[3][0] = -glm::dot(r, pos);
    m[3][1] = -glm::dot(u, pos);
    m[3][2] = glm::dot(f, pos);
    return m;
}

} // namespace common::utility
