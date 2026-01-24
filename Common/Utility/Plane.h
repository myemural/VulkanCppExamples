/**
 * @file    Plane.h
 * @brief   Container class for plane geometry.
 * @author  Mustafa Yemural (myemural)
 * @date    22.01.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "CoreDefines.h"

namespace common::utility
{

struct COMMON_API Plane
{
    glm::vec3 position;
    glm::vec3 normal;

    [[nodiscard]] glm::vec4 GetPlaneEquation() const;

    [[nodiscard]] glm::mat4 BuildReflectionMatrix() const;
};

} // namespace common::utility
