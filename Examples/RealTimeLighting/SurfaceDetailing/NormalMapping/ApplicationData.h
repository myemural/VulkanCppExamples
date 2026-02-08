/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    04.01.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::real_time_lighting::surface_detailing::normal_mapping
{

struct alignas(16) MeshMaterialData
{
    glm::vec4 diffuseColor = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 specularColor = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    float ambientStrength;
    float shininess;
    float specularStrength;
    int diffuseMap;
    int normalMap;
};

struct alignas(16) LightUbo
{
    glm::vec4 lightDirection; // xyz = Light Direction
    glm::vec4 lightColor;     // xyz = Light Color
};

struct MeshPushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
};

} // namespace examples::real_time_lighting::surface_detailing::normal_mapping
