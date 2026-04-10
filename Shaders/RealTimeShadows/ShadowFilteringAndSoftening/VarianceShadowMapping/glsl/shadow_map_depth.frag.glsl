#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------
layout(location = 0) out vec2 outMoments;

layout(push_constant) uniform MeshPushConstants {
    mat4 lightSpaceMatrix;
    uint objectId;
} pc;

void main()
{
    float depth = gl_FragCoord.z * 0.5 + 0.5;
    float moment1 = depth;
    float moment2 = depth * depth;
    outMoments = vec2(moment1, moment2);
}