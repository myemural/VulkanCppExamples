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

namespace examples::real_time_lighting::basic_lighting::specular_lighting_phong
{

struct LightUbo
{
    alignas(16) glm::vec3 lightPosition;
    float _pad0;
    alignas(16) glm::vec3 lightColor;
    float ambientStrength;
    float specularStrength;
    float shininess;
};

} // namespace examples::real_time_lighting::basic_lighting::specular_lighting_phong
