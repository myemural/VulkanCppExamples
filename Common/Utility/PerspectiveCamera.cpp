/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "PerspectiveCamera.h"

#include <glm/ext/matrix_clip_space.hpp>

namespace common::utility
{
PerspectiveCamera::PerspectiveCamera(glm::vec3 position, float aspect, float fov, float nearPlane, float farPlane)
    : CameraBase(position, aspect, nearPlane, farPlane), fov_(fov)
{
}

glm::mat4 PerspectiveCamera::GetProjectionMatrix() const
{
    return glm::perspective(glm::radians(fov_), aspect_, nearPlane_, farPlane_);
}

void PerspectiveCamera::SetFov(const float fov) { fov_ = fov; }

float PerspectiveCamera::GetFov() const { return fov_; }
} // namespace common::utility
