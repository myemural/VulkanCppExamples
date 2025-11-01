/**
 * @file    CameraBase.h
 * @brief   Base class for camera object.
 * @author  Mustafa Yemural (myemural)
 * @date    4.10.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "CoreDefines.h"

namespace common::utility
{

class COMMON_API CameraBase
{
public:
    /**
     * @param position Position of the camera.
     * @param aspect Aspect rate of the camera.
     * @param nearPlane Near plane distance of the camera.
     * @param farPlane Far plane distance of the camera.
     */
    CameraBase(glm::vec3 position, float aspect, float nearPlane, float farPlane);

    virtual ~CameraBase() = default;

    /**
     * @brief Returns calculated projection matrix of the camera.
     * @return Returns calculated projection matrix of the camera.
     */
    [[nodiscard]] virtual glm::mat4 GetProjectionMatrix() const = 0;

    /**
     * @brief Returns calculated view matrix of the camera.
     * @return Returns calculated view matrix of the camera.
     */
    [[nodiscard]] glm::mat4 GetViewMatrix() const;

    /**
     * @brief Sets position of the camera.
     * @param position New position of the camera.
     */
    void SetPosition(const glm::vec3& position);

    /**
     * @brief Returns position of the camera.
     * @return Returns position of the camera.
     */
    [[nodiscard]] glm::vec3 GetPosition() const;

    /**
     * @brief Sets aspect rate of the camera.
     * @param aspect New aspect rate of the camera.
     */
    void SetAspect(float aspect);

    /**
     * @brief Sets aspect rate of the camera with using width and height info.
     * @param width Width of the screen space.
     * @param height Height of the screen space.
     */
    void SetAspect(float width, float height);

    /**
     * @brief Returns aspect rate of the camera.
     * @return Returns aspect rate of the camera.
     */
    [[nodiscard]] float GetAspect() const;

    /**
     * @brief Sets near plane distance of the camera.
     * @param nearPlane New near plane distance of the camera.
     */
    void SetNearPlane(float nearPlane);

    /**
     * @brief Returns near plane distance of the camera.
     * @return Returns near plane distance of the camera.
     */
    [[nodiscard]] float GetNearPlane() const;

    /**
     * @brief Sets far plane distance of the camera.
     * @param farPlane New far plane distance of the camera.
     */
    void SetFarPlane(float farPlane);

    /**
     * @brief Returns far plane distance of the camera.
     * @return Returns far plane distance of the camera.
     */
    [[nodiscard]] float GetFarPlane() const;

    /**
     * @brief Sets new rotation of the camera.
     * @param yaw New yaw angle.
     * @param pitch New pitch angle.
     */
    void SetRotation(float yaw, float pitch);

    /**
     * @brief Returns yaw rotation value of the camera (in degree).
     * @return Returns yaw rotation value of the camera (in degree).
     */
    [[nodiscard]] float GetRotationYaw() const;

    /**
     * @brief Returns pitch rotation value of the camera (in degree).
     * @return Returns pitch rotation value of the camera (in degree).
     */
    [[nodiscard]] float GetRotationPitch() const;

    /**
     * @brief Returns front vector of the camera.
     * @return Returns front vector of the camera.
     */
    [[nodiscard]] glm::vec3 GetFrontVector() const;

    /**
     * @brief Returns right vector of the camera.
     * @return Returns right vector of the camera.
     */
    [[nodiscard]] glm::vec3 GetRightVector() const;

    /**
     * @brief Returns up vector of the camera.
     * @return Returns up vector of the camera.
     */
    [[nodiscard]] glm::vec3 GetUpVector() const;

    /**
     * @brief Sets new up vector of the world coordinate system.
     * @param worldUp Up vector of the world coordinate system.
     */
    void SetWorldUp(const glm::vec3& worldUp);

    /**
     * @brief Returns the current up vector of the world coordinate system.
     * @return Returns the current up vector of the world coordinate system.
     */
    [[nodiscard]] glm::vec3 GetWorldUp() const;

    /**
     * @brief Moves camera with a delta position value.
     * @param posDelta Delta position of the movement.
     */
    void Move(const glm::vec3& posDelta);

    /**
     * @brief Rotates camera with a delta yaw and a delta pitch angle values.
     * @param yawDelta Delta yaw angle of the rotation.
     * @param yawDelta Delta pitch angle of the rotation.
     */
    void Rotate(float yawDelta, float pitchDelta);

protected:
    glm::vec3 position_;
    float aspect_;
    float nearPlane_;
    float farPlane_;
    float yaw_;
    float pitch_;

private:
    void UpdateVectors();

    glm::vec3 cameraFront_ = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp_ = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraRight_ = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 worldUp_ = glm::vec3(0.0f, 1.0f, 0.0f);
};

} // namespace common::utility
