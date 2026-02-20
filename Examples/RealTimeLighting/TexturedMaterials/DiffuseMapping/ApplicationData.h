/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    21.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

namespace examples::real_time_lighting::textured_materials::diffuse_mapping
{

struct alignas(16) MeshMaterialData
{
    glm::vec4 diffuseColor = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 specularColor = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    float ambientStrength = 0.1f;
    float shininess = 32.0f;
    float specularStrength = 1.0f;
    std::int32_t diffuseMap = -1;
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

} // namespace examples::real_time_lighting::textured_materials::diffuse_mapping
