#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(push_constant) uniform Push {
    vec2 offset;
    int  samplerIndex;
} pc;

layout(location = 0) out vec2 fragUV;
layout(location = 1) flat out int samplerIdx;

void main()
{
    vec2 pos = inPosition + pc.offset;
    gl_Position = vec4(pos, 0.0, 1.0);
    fragUV = inUV;
    samplerIdx = pc.samplerIndex;
}