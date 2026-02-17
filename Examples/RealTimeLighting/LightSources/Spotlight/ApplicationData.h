/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    16.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Vertex.h"

namespace examples::real_time_lighting::light_sources::spotlight
{
#define LIGHT_COUNT 2

struct alignas(16) MeshMaterialData
{
    glm::vec4 diffuseColor = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 specularColor = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    float ambientStrength;
    float shininess;
    float specularStrength;
};

struct alignas(16) LightUbo
{
    glm::vec4 lightPosition;   // xyz = Light Position
    glm::vec4 lightDirection;  // xyz = Light Direction (normalized)
    glm::vec4 lightColor;      // xyz = Light Color
    glm::vec4 spotlightParams; // x = cos(cutoffAngle)
};

struct alignas(16) LightBlock
{
    LightUbo lightUbo[LIGHT_COUNT];
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
};

} // namespace examples::real_time_lighting::light_sources::spotlight
