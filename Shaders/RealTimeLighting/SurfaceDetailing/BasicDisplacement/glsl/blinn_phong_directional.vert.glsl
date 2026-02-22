#version 450
#extension GL_EXT_nonuniform_qualifier : require

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
layout(location = 2) out mat3 fragTBN;

struct MeshTransformData
{
    mat4 model;
    mat4 normalMatrix;
};

layout(std430, binding = 0) readonly buffer MeshTransformDataBuffer {
    MeshTransformData meshTransforms[];
};

struct MeshMaterialData
{
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    int diffuseMap;
    int normalMap;
    int heightMap;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

const float DISPLACEMENT_SCALE = 0.05; // For debugging

void main()
{
    // Get mesh data
    const MeshTransformData meshTransform = meshTransforms[pc.objectId];
    const MeshMaterialData meshMaterial = meshMaterials[pc.objectId];

    // Sample displacement (object space)
    float height = 0.0;
    if (meshMaterial.heightMap != -1) {
        height = texture(uCombinedSamplers[nonuniformEXT(meshMaterial.heightMap)], inUV).r;
    }

    // Displace vertex along normal (object space)
    vec3 displacedPosition = inPosition + inNormal * height * DISPLACEMENT_SCALE;

    // World-space position and UV
    vec4 worldPos = meshTransform.model * vec4(displacedPosition, 1.0);
    fragPos = worldPos.xyz;
    fragUv  = inUV;

    // Normal matrix
    mat3 normalMatrix = mat3(meshTransform.normalMatrix);

    // World-space normal and tangent
    vec3 N = normalize(normalMatrix * inNormal);
    vec3 T = normalize(normalMatrix * inTangent.xyz);

    // Orthonormalize T
    T = normalize(T - N * dot(N, T));

    // Bitangent calculation (multiplying with bitangent sign value)
    vec3 B = cross(N, T) * inTangent.w;

    fragTBN = mat3(T, B, N);

    gl_Position = pc.proj * pc.view * worldPos;
}