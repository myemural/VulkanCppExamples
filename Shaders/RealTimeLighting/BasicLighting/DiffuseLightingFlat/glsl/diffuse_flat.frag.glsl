#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) flat in vec3 fragNormal;

struct MeshData {
    mat4 model;
    vec4 objectColor;
};

layout(std430, set = 0, binding = 0) readonly buffer MeshDataBuffer {
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
    uint objectId;
} pc;

void main()
{
    // Normalizing light direction
    vec3 normalizedLightDir = normalize(light.lightPosition.xyz - fragPos);

    // Lambert diffuse
    float diff = max(dot(fragNormal, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * meshes[pc.objectId].objectColor.rgb;

    outColor = vec4(diffuse, 1.0);
}