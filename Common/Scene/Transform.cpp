/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "Transform.h"

namespace common::scene
{

void Transform::SetPosition(const glm::vec3& pos)
{
    position_ = pos;
    MarkDirty();
}
void Transform::SetEulerAngles(const glm::vec3& eulerDegrees)
{
    eulerAngles_ = eulerDegrees;
    MarkDirty();
}

void Transform::SetScale(const glm::vec3& scale)
{
    scale_ = scale;
    MarkDirty();
}

const glm::mat4& Transform::GetLocalMatrix()
{
    if (dirty_) {
        Recalculate();
    }

    return localMatrix_;
}

void Transform::Recalculate()
{
    const glm::vec3 radians = glm::radians(eulerAngles_);
    rotationQuat_ = glm::quat(radians);

    localMatrix_ = glm::translate(glm::mat4(1.0f), position_) * glm::mat4_cast(rotationQuat_) *
                   glm::scale(glm::mat4(1.0f), scale_);

    dirty_ = false;
}

} // namespace common::scene
