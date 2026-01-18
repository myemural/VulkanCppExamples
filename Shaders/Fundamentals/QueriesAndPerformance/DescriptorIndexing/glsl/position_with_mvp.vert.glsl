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
layout(location = 1) flat out uint textureIndex;

layout(set = 0, binding = 0) uniform ObjectUBO {
    mat4 mvpMatrix;
    uint textureIndex;
} ubo;

void main()
{
    fragUV = inUV;
    textureIndex = ubo.textureIndex;
    gl_Position = ubo.mvpMatrix * vec4(inPosition, 1.0);
}