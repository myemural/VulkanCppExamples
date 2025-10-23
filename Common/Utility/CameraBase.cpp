/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "CameraBase.h"

#include <cmath>

#include <glm/ext/matrix_transform.hpp>

namespace common::utility
{
CameraBase::CameraBase(glm::vec3 position, float aspect, float nearPlane, float farPlane)
    : position_(position), aspect_(aspect), nearPlane_(nearPlane), farPlane_(farPlane), yaw_(-90.0f), pitch_(0.0f)
{
    UpdateVectors();
}

glm::mat4 CameraBase::GetViewMatrix() const { return glm::lookAt(position_, position_ + cameraFront_, cameraUp_); }

void CameraBase::SetPosition(const glm::vec3& position) { position_ = position; }

glm::vec3 CameraBase::GetPosition() const { return position_; }

void CameraBase::SetAspect(const float aspect) { aspect_ = aspect; }

void CameraBase::SetAspect(const float width, const float height) { aspect_ = width / height; }

float CameraBase::GetAspect() const { return aspect_; }

void CameraBase::SetNearPlane(const float nearPlane) { nearPlane_ = nearPlane; }

float CameraBase::GetNearPlane() const { return nearPlane_; }

void CameraBase::SetFarPlane(const float farPlane) { farPlane_ = farPlane; }

float CameraBase::GetFarPlane() const { return farPlane_; }

void CameraBase::SetRotation(const float yaw, const float pitch)
{
    yaw_ = yaw;
    pitch_ = glm::clamp(pitch, -89.0f, 89.0f);
    UpdateVectors();
}

float CameraBase::GetRotationYaw() const { return yaw_; }

float CameraBase::GetRotationPitch() const { return pitch_; }

glm::vec3 CameraBase::GetFrontVector() const { return cameraFront_; }

glm::vec3 CameraBase::GetRightVector() const { return cameraRight_; }

glm::vec3 CameraBase::GetUpVector() const { return cameraUp_; }

void CameraBase::SetWorldUp(const glm::vec3& worldUp)
{
    worldUp_ = worldUp;
    UpdateVectors();
}

glm::vec3 CameraBase::GetWorldUp() const { return worldUp_; }

void CameraBase::Move(const glm::vec3& posDelta) { position_ += posDelta; }

void CameraBase::Rotate(const float yawDelta, const float pitchDelta)
{
    yaw_ += yawDelta;
    pitch_ = glm::clamp(pitch_ + pitchDelta, -89.0f, 89.0f);
    UpdateVectors();
}

void CameraBase::UpdateVectors()
{
    glm::vec3 front;
    front.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    front.y = std::sin(glm::radians(pitch_));
    front.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    cameraFront_ = glm::normalize(front);

    cameraRight_ = glm::normalize(glm::cross(cameraFront_, worldUp_));
    cameraUp_ = glm::normalize(glm::cross(cameraRight_, cameraFront_));
}
} // namespace common::utility
