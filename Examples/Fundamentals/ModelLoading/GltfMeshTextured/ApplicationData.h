/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    13.10.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include <glm/mat4x4.hpp>

#include "ModelLoader.h"
#include "VulkanHelpers.h"

namespace examples::fundamentals::model_loading::gltf_mesh_textured
{
#define NUM_OBJECTS 10

// MVP Matrices (for Push Constants)
struct MvpData
{
    glm::mat4 mvpMatrix;
};

// Model position vectors
inline const std::vector<glm::vec3> modelPositions = common::vulkan_framework::GenerateRandomPositions(
        NUM_OBJECTS, glm::vec3(-3.0f, -3.0f, -4.0f), glm::vec3(3.0f, 3.0f, -1.0f), 15.0f);
} // namespace examples::fundamentals::model_loading::gltf_mesh_textured
