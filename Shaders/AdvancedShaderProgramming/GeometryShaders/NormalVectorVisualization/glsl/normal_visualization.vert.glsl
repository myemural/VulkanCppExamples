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

layout(location = 0) out vec3 geomNormal;

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
    // Get mesh data
    const MeshTransformData meshTransform = meshTransforms[pc.objectId];

    // Set position in view-space
    gl_Position = pc.view * meshTransform.model * vec4(inPosition, 1.0);

    // Set geometric normal in view-space
    mat3 normalMatrix = mat3(pc.view) * mat3(meshTransform.normalMatrix);
    geomNormal = normalize(vec3(vec4(normalMatrix * inNormal, 0.0)));
}