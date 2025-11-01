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

#include <glm/glm.hpp>

#include "CameraBase.h"
#include "CoreDefines.h"

namespace common::utility
{

class COMMON_API OrthographicCamera final : public CameraBase
{
public:
    /**
     * @param position Position vector of the camera.
     * @param aspect Aspect ratio of the camera.
     * @param size Size of the orthographic frustum.
     * @param nearPlane Near plane distance of the orthographic frustum.
     * @param farPlane Far plane distance of the orthographic frustum.
     */
    OrthographicCamera(
            glm::vec3 position, float aspect, float size = 10.0f, float nearPlane = 0.1f, float farPlane = 100.0f);

    /**
     * @brief Returns calculated projection matrix of the camera.
     * @return Returns calculated projection matrix of the camera.
     */
    [[nodiscard]] glm::mat4 GetProjectionMatrix() const override;

    /**
     * @brief Sets the size value of the camera.
     * @param size New size value.
     */
    void SetSize(float size);

    /**
     * @brief Returns the current size value of the camera.
     * @return Returns the current size value of the camera.
     */
    [[nodiscard]] float GetSize() const;

private:
    float size_;
};

} // namespace common::utility
