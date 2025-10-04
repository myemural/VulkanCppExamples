/**
 * @file    PerspectiveCamera.h
 * @brief   Perspective camera object implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    4.10.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "CameraBase.h"

#include <glm/glm.hpp>

namespace common::utility
{

class PerspectiveCamera final : public CameraBase
{
public:
    PerspectiveCamera(
            glm::vec3 position, float aspect, float fov = 45.0f, float nearPlane = 0.1f, float farPlane = 100.0f);

    [[nodiscard]] glm::mat4 GetProjectionMatrix() const override;

    void SetFov(float fov);
    [[nodiscard]] float GetFov() const;

private:
    float fov_;
};

} // namespace common::utility
