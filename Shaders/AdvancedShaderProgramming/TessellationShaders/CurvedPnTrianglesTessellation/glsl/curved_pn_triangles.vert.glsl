#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec4 outTangent;

void main()
{
    outPosition = inPosition;
    outUV = inUV;
    outNormal = inNormal;
    outTangent = inTangent;
}
