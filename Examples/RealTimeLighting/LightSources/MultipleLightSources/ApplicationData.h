/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    18.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "Vertex.h"

namespace examples::real_time_lighting::light_sources::multiple_light_sources
{
#define LIGHT_COUNT 4

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct alignas(16) LightData
{
    glm::vec4 lightPosition;    // xyz = Light Position (unused for directional lights)
    glm::vec4 lightDirection;   // xyz = Light Direction (normalized, unused for point lights)
    glm::vec4 lightColor;       // rgb = Light Color, a = Light Intensity

    glm::vec4 lightTypeParams;  // x = Light Type (0: Directional, 1: Point: 2: Spot)
    glm::vec4 pointLightParams; // x = Constant Factor, y = Linear Factor, z = Quadratic Factor
    glm::vec4 spotlightParams;  // x = cos(innerCutoffAngle), y = cos(outerCutoffAngle)
};

struct LightBuffer
{
    LightData lights[LIGHT_COUNT];
};

} // namespace examples::real_time_lighting::light_sources::multiple_light_sources
