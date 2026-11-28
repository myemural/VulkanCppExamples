#version 460
#extension GL_EXT_ray_tracing : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) rayPayloadInEXT vec3 payloadColor;

// Barycentric coordinates of the hit point for vertex 1 and vertex 2
hitAttributeEXT vec2 hitBarycentrics;

const vec3 kVertexColors[3] = vec3[](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

void main()
{
    const vec3 barycentrics = vec3(1.0 - hitBarycentrics.x - hitBarycentrics.y, hitBarycentrics.x, hitBarycentrics.y);

    payloadColor = kVertexColors[0] * barycentrics.x + kVertexColors[1] * barycentrics.y +
    kVertexColors[2] * barycentrics.z;
}
