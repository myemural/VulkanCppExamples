#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) flat in vec3 fragNormal;

struct MeshTransformData
{
    mat4 model;
    mat4 normalMatrix;
};

layout(std430, binding = 0) readonly buffer MeshTransformDataBuffer {
    MeshTransformData meshTransforms[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

void main()
{
    // Hardcoded white color for light objects
    outColor = vec4(1.0, 1.0, 1.0, 1.0);
}