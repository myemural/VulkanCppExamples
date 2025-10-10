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
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    fragUV = inUV;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
}