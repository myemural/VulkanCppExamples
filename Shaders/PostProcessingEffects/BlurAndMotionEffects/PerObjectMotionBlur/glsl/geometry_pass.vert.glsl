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
layout(location = 5) out vec4 fragCurrentClip;  // Clip-space position in the current frame
layout(location = 6) out vec4 fragPreviousClip; // Clip-space position in the previous frame

struct MeshTransformData
{
    mat4 model;
    mat4 normalMatrix;
};

layout(std430, binding = 0) readonly buffer MeshTransformDataBuffer {
    MeshTransformData meshTransforms[];
};

// Matrices of the previous frame (for the velocity buffer)
layout(std430, binding = 3) readonly buffer PreviousFrameDataBuffer {
    mat4 previousViewProjection;
    mat4 previousModels[]; // Indexed with the objectId
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    uint objectId;
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

    // Same vertex with the current and pevious frames's matrices (object motion and camera motion together)
    fragCurrentClip = gl_Position;
    fragPreviousClip = previousViewProjection * previousModels[pc.objectId] * vec4(inPosition, 1.0);
}
