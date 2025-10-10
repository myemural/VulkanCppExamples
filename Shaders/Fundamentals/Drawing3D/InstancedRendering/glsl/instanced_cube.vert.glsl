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

layout(location = 0) out vec2 fragUV;

layout(set = 0, binding = 1) uniform UBO {
    mat4 mvp[20];
} ubo;

void main()
{
    fragUV = inUV;
    gl_Position = ubo.mvp[gl_InstanceIndex] * vec4(inPosition, 1.0);
}