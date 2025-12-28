/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    28.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Vertex.h"
#include "VulkanHelpers.h"

namespace examples::real_time_lighting::texture_sampling_and_filtering::anisotropic_filtering
{

#define NUM_OBJECTS 50

struct alignas(16) LightUbo
{
    glm::vec4 lightDirection; // xyz = Light Direction
    glm::vec4 lightColor;     // xyz = Light Color
};

// Model position vectors
inline const std::vector<glm::vec3> modelPositions = common::vulkan_framework::GenerateRandomPositions(
        NUM_OBJECTS, glm::vec3(-6.0f, -6.0f, -14.0f), glm::vec3(6.0f, 6.0f, -2.0f), 1.5f);

} // namespace examples::real_time_lighting::texture_sampling_and_filtering::anisotropic_filtering
