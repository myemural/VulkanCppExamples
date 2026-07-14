#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) in vec3 fragPos;

layout(push_constant) uniform MeshPushConstants {
    vec4 lightPos;
    uint objectId;
    float farPlane;
} pc;

void main()
{
    float lightDistance = length(fragPos - pc.lightPos.xyz);
    gl_FragDepth = lightDistance / pc.farPlane;
}