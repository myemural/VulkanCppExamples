#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------
layout(location = 0) out vec4 outMoments;

layout(push_constant) uniform MeshPushConstants {
    mat4 lightSpaceMatrix;
    uint objectId;
    float esmExponent;
} pc;

void main()
{
    float depth = gl_FragCoord.z;

    // Exponential warp
    float pos = exp(pc.esmExponent * depth);
    float neg = exp(-pc.esmExponent * depth);

    outMoments = vec4(pos, pos * pos, neg, neg * neg);
}