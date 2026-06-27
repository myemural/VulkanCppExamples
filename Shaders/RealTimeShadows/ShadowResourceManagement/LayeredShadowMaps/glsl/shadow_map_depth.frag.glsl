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
    mat4 lightSpaceMatrix;
    uint objectId;
} pc;

void main()
{
    gl_FragDepth = gl_FragCoord.z;
}