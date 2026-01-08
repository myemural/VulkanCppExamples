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
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out mat3 fragTBN;

struct MeshData {
    mat4 model;
    mat4 normalMatrix;
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float opacity;
    int diffuseMap;
    int specularMap;
    int normalMap;
    int emissiveMap;
    int shininessMap;
    int opacityMap;
    int aoMap;
    int heightMap;
};

layout(std430, binding = 0) readonly buffer MeshDataBuffer {
    MeshData meshes[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

void main()
{
    // Get mesh data
    MeshData mesh = meshes[pc.objectId];

    // World-space position and UV
    fragPos = vec3(mesh.model * vec4(inPosition, 1.0));
    fragUv  = inUV;

    // Normal matrix
    mat3 normalMatrix = mat3(mesh.normalMatrix);

    // World-space normal and tangent
    vec3 N = normalize(normalMatrix * inNormal);
    vec3 T = normalize(normalMatrix * inTangent);

    // Orthonormalize T
    T = normalize(T - N * dot(N, T));

    // Bitangent calculation
    vec3 B = cross(N, T);

    fragTBN = mat3(T, B, N);

    gl_Position = pc.proj * pc.view * vec4(fragPos, 1.0);
}