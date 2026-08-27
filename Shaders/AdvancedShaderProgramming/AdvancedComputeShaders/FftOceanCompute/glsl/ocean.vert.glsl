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
layout(location = 1) out vec2 fragUv;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out float fragFoam;

struct MeshTransformData
{
    mat4 model;
    mat4 normalMatrix;
};

layout(std430, binding = 0) readonly buffer MeshTransformDataBuffer {
    MeshTransformData meshTransforms[];
};

struct OceanVertex
{
    vec4 displacement; // xyz = World-Space Displacement, w = Foam Factor
    vec4 normal;       // xyz = World-Space Normal
};

layout(std430, binding = 5) readonly buffer OceanVertexBuffer {
    OceanVertex oceanVertices[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

void main()
{
    const MeshTransformData meshTransform = meshTransforms[pc.objectId];
    const OceanVertex oceanVertex = oceanVertices[gl_VertexIndex];

    const vec3 basePosition = vec3(meshTransform.model * vec4(inPosition, 1.0));
    fragPos = basePosition + oceanVertex.displacement.xyz; // Apply ocean displacement
    fragUv  = inUV;
    fragNormal  = oceanVertex.normal.xyz;
    fragFoam = oceanVertex.displacement.w;

    gl_Position = pc.proj * pc.view * vec4(fragPos, 1.0);
}
