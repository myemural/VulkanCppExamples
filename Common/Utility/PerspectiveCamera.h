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

#include <glm/glm.hpp>

#include "CameraBase.h"
#include "CoreDefines.h"

namespace common::utility
{

class COMMON_API PerspectiveCamera final : public CameraBase
{
public:
    /**
     * @param position Position vector of the camera.
     * @param aspect Aspect ratio of the camera.
     * @param fov Fov angle of the perspective frustum (in degree).
     * @param nearPlane Near plane distance of the perspective frustum.
     * @param farPlane Far plane distance of the perspective frustum.
     */
    PerspectiveCamera(
            glm::vec3 position, float aspect, float fov = 45.0f, float nearPlane = 0.1f, float farPlane = 100.0f);

    /**
     * @brief Returns calculated projection matrix of the camera.
     * @return Returns calculated projection matrix of the camera.
     */
    [[nodiscard]] glm::mat4 GetProjectionMatrix() const override;

    /**
     * @brief Sets the FOV value of the camera.
     * @param fov New FOV value.
     */
    void SetFov(float fov);

    /**
     * @brief Returns the current FOV value of the camera.
     * @return Returns the current FOV value of the camera.
     */
    [[nodiscard]] float GetFov() const;

private:
    float fov_;
};

} // namespace common::utility
