/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "OrthographicCamera.h"

#include <glm/ext/matrix_clip_space.hpp>

namespace common::utility
{
OrthographicCamera::OrthographicCamera(glm::vec3 position, float aspect, float size, float nearPlane, float farPlane)
    : CameraBase(position, aspect, nearPlane, farPlane), size_(size)
{
}

glm::mat4 OrthographicCamera::GetProjectionMatrix() const
{
    const float halfW = size_ * aspect_ * 0.5f;
    const float halfH = size_ * 0.5f;
    return glm::ortho(-halfW, halfW, -halfH, halfH, nearPlane_, farPlane_);
}

void OrthographicCamera::SetSize(const float size) { size_ = size; }

float OrthographicCamera::GetSize() const { return size_; }


} // namespace common::utility
