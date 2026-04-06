/**
 * @file    Transform.h
 * @brief   Contains transform class implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    07.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include "CoreDefines.h"

namespace common::scene
{

class COMMON_API Transform
{
public:
    Transform() = default;
    ~Transform() = default;

    void SetPosition(const glm::vec3& pos);

    void SetEulerAngles(const glm::vec3& eulerDegrees);

    void SetQuaternion(const glm::vec4& quat);

    void SetScale(const glm::vec3& scale);

    const glm::mat4& GetLocalMatrix();

    [[nodiscard]] glm::vec3 GetPosition() const { return position_; }
    [[nodiscard]] glm::vec3 GetEulerAngles() const { return eulerAngles_; }
    [[nodiscard]] glm::vec3 GetScale() const { return scale_; }

    [[nodiscard]] bool IsDirty() const { return dirty_; }
    void ClearDirty() { dirty_ = false; }
    void MarkDirty() { dirty_ = true; }

private:
    void Recalculate();

    glm::vec3 position_{0.0f};
    glm::vec3 eulerAngles_{0.0f}; // pitch (X), yaw (Y), roll (Z)
    glm::vec3 scale_{1.0f};

    glm::quat rotationQuat_{1, 0, 0, 0};
    glm::mat4 localMatrix_{1.0f};
    bool dirty_ = true;
    bool skipEulerAngles_ = false;
};

struct COMMON_API TransformGpu
{
    glm::mat4 modelMatrix;
    glm::mat4 normalMatrix;
};

} // namespace common::scene
