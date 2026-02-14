#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;

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
    uint objectId;
} pc;

void main()
{
    mat4 model = meshTransforms[pc.objectId].model;

    fragPos = vec3(model * vec4(inPosition, 1.0));

    mat3 normalMatrix = mat3(meshTransforms[pc.objectId].normalMatrix);
    fragNormal = normalMatrix * inNormal;

    gl_Position = pc.proj * pc.view * vec4(fragPos, 1.0);
}