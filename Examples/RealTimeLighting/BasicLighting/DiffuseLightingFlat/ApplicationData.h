/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    13.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Vertex.h"

namespace examples::real_time_lighting::basic_lighting::diffuse_lighting_flat
{

struct alignas(16) LightUbo
{
    glm::vec4 lightPosition; // xyz = Light Position
    glm::vec4 lightColor;    // xyz = Light Color
};

} // namespace examples::real_time_lighting::basic_lighting::diffuse_lighting_flat
