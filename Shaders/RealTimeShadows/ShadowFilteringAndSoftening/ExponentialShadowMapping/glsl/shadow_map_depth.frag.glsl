#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------
layout(location = 0) out float outDepth;

layout(push_constant) uniform MeshPushConstants {
    mat4 lightSpaceMatrix;
    uint objectId;
    float esmExponent;
} pc;

void main()
{
    float depth = gl_FragCoord.z;
    outDepth = exp(pc.esmExponent * depth);
}