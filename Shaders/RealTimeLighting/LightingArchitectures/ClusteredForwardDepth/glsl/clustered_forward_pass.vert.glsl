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

layout(location = 0) out vec3 fragPosView;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out mat3 fragTbnView;

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
    uint tilesX;
    uint objectId;
    float nearPlane;
    float farPlane;
} pc;

void main()
{
    // Get mesh data
    MeshTransformData meshTransform = meshTransforms[pc.objectId];

    // World-space position and UV
    vec4 worldPos = meshTransform.model * vec4(inPosition, 1.0);
    fragPosView = vec3(pc.view * worldPos);
    fragUv = inUV;

    // Normal matrix in view-space
    mat3 normalMatrixView = mat3(pc.view) * mat3(meshTransform.normalMatrix);

    // View-space normal and tangent
    vec3 N = normalize(normalMatrixView * inNormal);
    vec3 T = normalize(normalMatrixView * inTangent.xyz);

    // Orthonormalize T
    T = normalize(T - N * dot(N, T));

    // Bitangent calculation (multiplying with bitangent sign value)
    vec3 B = cross(N, T) * inTangent.w;

    fragTbnView = mat3(T, B, N);

    gl_Position = pc.proj * pc.view * worldPos;
}