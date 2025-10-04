/**
 * @file    OrthographicCamera.h
 * @brief   Orthographic camera object implementation.
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

class OrthographicCamera final : public CameraBase
{
public:
    OrthographicCamera(
            glm::vec3 position, float aspect, float size = 10.0f, float nearPlane = 0.1f, float farPlane = 100.0f);

    [[nodiscard]] glm::mat4 GetProjectionMatrix() const override;

    void SetSize(float size);
    [[nodiscard]] float GetSize() const;

private:
    float size_;
};

} // namespace common::utility
