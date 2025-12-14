/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    14.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Vertex.h"

namespace examples::real_time_lighting::light_sources::directional_light
{

struct alignas(16) LightUbo
{
    glm::vec4 lightDirection; // xyz = Light Direction
    glm::vec4 lightColor;     // xyz = Light Color
    glm::vec4 ambientParams;  // x = Ambient Strength
    glm::vec4 specularParams; // x = Specular Strength, y = Shininess
};

} // namespace examples::real_time_lighting::light_sources::directional_light
