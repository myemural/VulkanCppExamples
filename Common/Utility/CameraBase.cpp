/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "CameraBase.h"

#include <cmath>

#include <glm/ext/matrix_transform.hpp>

#include "MathUtils.h"

namespace common::utility
{
CameraBase::CameraBase(glm::vec3 position, float aspect, float nearPlane, float farPlane)
    : position_(position), aspect_(aspect), nearPlane_(nearPlane), farPlane_(farPlane), yaw_(-90.0f), pitch_(0.0f)
{
    UpdateVectors();
}

glm::mat4 CameraBase::GetViewMatrix() const { return glm::lookAt(position_, position_ + cameraFront_, cameraUp_); }

glm::mat4 CameraBase::GetReflectedViewMatrix(const Plane& plane) const
{
    const glm::mat4 view = GetViewMatrix();
    const glm::mat4 reflect = plane.BuildReflectionMatrix();

    return view * reflect;
}

glm::mat4 CameraBase::GetReflectionViewProjMatrix(const Plane& plane) const
{
    const glm::mat4 reflectedView = GetReflectedViewMatrix(plane);
    const glm::mat4 projection = GetProjectionMatrix();

    // Clip space [-1, 1] to texture space [0, 1] transform
    glm::mat4 clipToTextureSpace(1.0f);
    clipToTextureSpace = glm::translate(clipToTextureSpace, glm::vec3(0.5f, 0.5f, 0.0f));
    clipToTextureSpace = glm::scale(clipToTextureSpace, glm::vec3(0.5f, 0.5f, 1.0f));

    return clipToTextureSpace * projection * reflectedView;
}

std::array<glm::mat4, 6> CameraBase::GetCubemapViewMatrices() const
{
    const glm::vec3 pos = position_;
    std::array<glm::mat4, 6> views{};

    views[0] = MakeCubemapView(pos, {1, 0, 0}, {0, -1, 0});  // +X
    views[1] = MakeCubemapView(pos, {-1, 0, 0}, {0, -1, 0}); // -X

    views[2] = MakeCubemapView(pos, {0, 1, 0}, {0, 0, 1});   // +Y
    views[3] = MakeCubemapView(pos, {0, -1, 0}, {0, 0, -1}); // -Y

    views[4] = MakeCubemapView(pos, {0, 0, 1}, {0, -1, 0}); // +Z
    views[5] = MakeCubemapView(pos, {0, 0, -1}, {0, -1, 0});  // -Z

    return views;
}

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
