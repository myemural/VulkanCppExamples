/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "MathUtils.h"

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

} // namespace common::utility
