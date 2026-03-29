/**
 * @file    MathUtils.h
 * @brief   This file contains math related utility functions.
 * @author  Mustafa Yemural (myemural)
 * @date    8.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <cstdint>
#include <random>

#include <glm/glm.hpp>

#include "CoreDefines.h"

namespace common::utility
{

/**
 * @brief Computes the ceiling division between two integral values.
 * @param value The value to be divided.
 * @param divisor The divisor.
 * @return Divides value by divisor. If value is not a multiple of divisor rounds the result up to the next integer
 * value.
 */
COMMON_API std::uint32_t CeilDiv(std::uint32_t value, std::uint32_t divisor);

/**
 * @brief Generates random position for certain amount of objects within specific scene bounds and min distance.
 * @param count Number of objects.
 * @param minBounds Minimum bounds of the scene.
 * @param maxBounds Maximum bounds of the scene.
 * @param minDistance Minimum distance between objects.
 * @return Random positions for the objects.
 */
COMMON_API std::vector<glm::vec3>
GenerateRandomPositions(size_t count, glm::vec3 minBounds, glm::vec3 maxBounds, float minDistance = 1.5f);

/**
 * @brief Generates random position for certain amount of objects within specific bounds (corners, edges and faces).
 * @param count Number of objects.
 * @param minBounds Minimum bounds of the scene.
 * @param maxBounds Maximum bounds of the scene.
 * @param minDistance Minimum distance between objects.
 * @return Random positions for the objects.
 */
COMMON_API std::vector<glm::vec3>
GenerateRandomPositionsOnBounds(size_t count, glm::vec3 minBounds, glm::vec3 maxBounds, float minDistance = 1.5f);

/**
 * @brief Generates and returns random color value between min and max values. Alpha is not included.
 * @param minValue Minimum value per color component. Default is 0.
 * @param maxValue Maximum value for color component. Default is 1.
 * @return Returns random color value.
 */
COMMON_API glm::vec3 GenerateRandomColor(float minValue = 0.0f, float maxValue = 1.0f);

/**
 * @brief A concept type that encompasses both integers and floating-point numbers.
 */
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

/**
 * @brief Generates and returns random value between min and max values.
 * @param minValue Minimum value.
 * @param maxValue Maximum value.
 * @return Returns random value.
 */
template<Numeric T>
T GenerateRandomValue(const T minValue, const T maxValue)
{
    std::mt19937 generator{std::random_device{}()};
    if constexpr (std::integral<T>) {
        std::uniform_int_distribution<T> distribution(minValue, maxValue);
        return distribution(generator);
    } else {
        std::uniform_real_distribution<T> distribution(minValue, maxValue);
        return distribution(generator);
    }
}

/**
 * @brief Calculates mip level count to the texture image width and height.
 * @param textureWidth Width of the texture image.
 * @param textureHeight Height of the texture image.
 * @return Returns mip level count.
 */
COMMON_API std::uint32_t GetMipLevelCount(std::uint32_t textureWidth, std::uint32_t textureHeight);

/**
 * @brief Creates a view matrix for a single cubemap face.
 * @param pos World-space position of the cubemap camera.
 * @param dir Forward direction of the cubemap face (must be axis-aligned).
 * @param up Up vector defining the vertical orientation of the cubemap face.
 * @return View matrix for the specified cubemap face.
 */
COMMON_API glm::mat4 MakeCubemapView(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up);

} // namespace common::utility
