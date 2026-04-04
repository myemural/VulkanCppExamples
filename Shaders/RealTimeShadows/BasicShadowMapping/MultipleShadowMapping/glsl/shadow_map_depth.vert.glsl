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

layout(location = 0) out vec3 fragPos;

struct MeshTransformData
{
    mat4 model;
    mat4 normalMatrix;
};

layout(std430, binding = 0) readonly buffer MeshTransformDataBuffer {
    MeshTransformData meshTransforms[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 lightSpaceMatrix;
    vec4 lightPos;
    uint objectId;
    float farPlane;
    uint isPointLightShadow; // 0: False, 1: True
} pc;

void main()
{
    // Get mesh model matrix
    const mat4 meshModelMatrix = meshTransforms[pc.objectId].model;

    vec4 worldPos = meshModelMatrix * vec4(inPosition, 1.0);
    fragPos = worldPos.xyz;

    gl_Position = pc.lightSpaceMatrix * worldPos;
}