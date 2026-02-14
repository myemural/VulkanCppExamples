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

layout(location = 0) out vec3 vertexColor;

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
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightPosition; // xyz = Light Position
    vec4 lightColor;    // rgb = Light Color
} light;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    uint objectId;
} pc;

void main()
{
    // Get mesh info
    const MeshTransformData meshTransformInfo = meshTransforms[pc.objectId];
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    // World-space position
    vec3 worldPos = vec3(meshTransformInfo.model * vec4(inPosition, 1.0));

    // World-space normal
    mat3 normalMatrix = mat3(meshTransformInfo.normalMatrix);
    vec3 worldNormal = normalize(normalMatrix * inNormal);

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(light.lightPosition.xyz - worldPos);

    // Lambert diffuse
    float diff = max(dot(worldNormal, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * meshMatInfo.diffuseColor.rgb;
    vertexColor = diffuse;

    gl_Position = pc.proj * pc.view * vec4(worldPos, 1.0);
}