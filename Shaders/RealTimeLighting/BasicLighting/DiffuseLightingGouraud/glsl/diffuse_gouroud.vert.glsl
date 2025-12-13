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
    vec4 objectColor;
};

layout(std430, binding = 0) readonly buffer MeshDataBuffer {
    MeshData meshes[];
};

layout(set = 0, binding = 1) uniform LightUBO
{
    vec3 lightPosition;
    vec3 lightColor;
} light;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    uint objectId;
} pc;

void main()
{
    mat4 model = meshes[pc.objectId].model;

    // World-space position
    vec3 worldPos = vec3(model * vec4(inPosition, 1.0));

    // World-space normal
    vec3 worldNormal = normalize(transpose(inverse(mat3(model))) * inNormal);

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(light.lightPosition - worldPos);

    // Lambert diffuse
    float diff = max(dot(worldNormal, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor * meshes[pc.objectId].objectColor.rgb;
    vertexColor = diffuse;

    gl_Position = pc.proj * pc.view * vec4(worldPos, 1.0);
}