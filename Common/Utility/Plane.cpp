/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "Plane.h"

namespace common::utility
{

glm::vec4 Plane::GetPlaneEquation() const
{
    const glm::vec3 n = glm::normalize(normal);
    const float d = -glm::dot(n, position);
    return {n, d}; // (A, B, C, D)
}

glm::mat4 Plane::BuildReflectionMatrix() const
{
    const glm::vec4 planeEq = GetPlaneEquation();
    const auto n = glm::vec3(planeEq);
    const float d = planeEq.w;

    glm::mat4 R(1.0f);

    // Upper-left 3x3: I - 2 * n * n^T
    R[0][0] = 1.0f - 2.0f * n.x * n.x;
    R[0][1] = -2.0f * n.x * n.y;
    R[0][2] = -2.0f * n.x * n.z;

    R[1][0] = -2.0f * n.y * n.x;
    R[1][1] = 1.0f - 2.0f * n.y * n.y;
    R[1][2] = -2.0f * n.y * n.z;

    R[2][0] = -2.0f * n.z * n.x;
    R[2][1] = -2.0f * n.z * n.y;
    R[2][2] = 1.0f - 2.0f * n.z * n.z;

    // Translation: -2 * D * n
    R[3] = glm::vec4(-2.0f * d * n, 1.0f);

    return R;
}

} // namespace common::utility
