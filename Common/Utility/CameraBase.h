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

namespace common::utility
{

class CameraBase
{
public:
    CameraBase(glm::vec3 position, float aspect, float nearPlane, float farPlane);

    virtual ~CameraBase() = default;

    [[nodiscard]] virtual glm::mat4 GetProjectionMatrix() const = 0;

    [[nodiscard]] glm::mat4 GetViewMatrix() const;

    void SetPosition(const glm::vec3& position);
    [[nodiscard]] glm::vec3 GetPosition() const;

    void SetAspect(float aspect);
    void SetAspect(float width, float height);
    [[nodiscard]] float GetAspect() const;

    void SetNearPlane(float nearPlane);
    [[nodiscard]] float GetNearPlane() const;

    void SetFarPlane(float farPlane);
    [[nodiscard]] float GetFarPlane() const;

    void SetRotation(float yaw, float pitch);
    [[nodiscard]] float GetRotationYaw() const;
    [[nodiscard]] float GetRotationPitch() const;

    [[nodiscard]] glm::vec3 GetFrontVector() const;
    [[nodiscard]] glm::vec3 GetRightVector() const;
    [[nodiscard]] glm::vec3 GetUpVector() const;

    void SetWorldUp(const glm::vec3& worldUp);
    [[nodiscard]] glm::vec3 GetWorldUp() const;

    void Move(const glm::vec3& posDelta);

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
