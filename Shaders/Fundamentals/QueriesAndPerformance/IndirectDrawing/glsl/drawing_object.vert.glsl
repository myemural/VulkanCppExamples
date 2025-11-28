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

layout(set = 0, binding = 0) readonly buffer ObjectSSBO {
    mat4 mvpMatrix[];
} ssbo;

void main()
{
    fragUV = inUV;
    gl_Position = ssbo.mvpMatrix[gl_InstanceIndex] * vec4(inPosition, 1.0);
}