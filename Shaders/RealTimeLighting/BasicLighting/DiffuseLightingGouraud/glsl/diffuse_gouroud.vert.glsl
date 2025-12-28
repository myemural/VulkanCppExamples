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

struct MeshData {
    mat4 model;
    mat4 normalMatrix;
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float opacity;
};

layout(std430, binding = 0) readonly buffer MeshDataBuffer {
    MeshData meshes[];
};

layout(std140, set = 0, binding = 1) uniform LightUBO
{
    vec4 lightPosition; // xyz = Light Position
    vec4 lightColor;    // rgb = Light Color
} light;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

void main()
{
    // Get mesh info
    const MeshData meshInfo = meshes[pc.objectId];

    // World-space position
    vec3 worldPos = vec3(meshInfo.model * vec4(inPosition, 1.0));

    // World-space normal
    mat3 normalMatrix = mat3(meshes[pc.objectId].normalMatrix);
    vec3 worldNormal = normalize(normalMatrix * inNormal);

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(light.lightPosition.xyz - worldPos);

    // Lambert diffuse
    float diff = max(dot(worldNormal, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * meshInfo.diffuseColor.rgb;
    vertexColor = diffuse;

    gl_Position = pc.proj * pc.view * vec4(worldPos, 1.0);
}