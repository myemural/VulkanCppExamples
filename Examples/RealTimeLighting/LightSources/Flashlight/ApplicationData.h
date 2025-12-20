/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    17.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Vertex.h"

namespace examples::real_time_lighting::light_sources::flashlight
{

struct alignas(16) LightUbo
{
    glm::vec4 lightPosition;    // xyz = Light Position
    glm::vec4 lightDirection;   // xyz = Light Direction (normalized)
    glm::vec4 lightColor;       // xyz = Light Color
    glm::vec4 spotlightParams;  // x = cos(innerCutoffAngle), y = cos(outerCutoffAngle)
};

} // namespace examples::real_time_lighting::light_sources::flashlight
