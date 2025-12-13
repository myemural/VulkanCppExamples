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
layout(location = 1) in vec3 fragNormal;

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
    float ambientStrength;
    float specularStrength;
    float shininess;
} light;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    uint objectId;
} pc;

void main()
{
    // Normalizing normal
    vec3 normalizedNormal = normalize(fragNormal);

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(light.lightPosition - fragPos);

    // Normalizing view direction (camera position)
    mat4 inverseView = inverse(pc.view);
    vec3 viewPos = vec3(inverseView[3]);
    vec3 normalizedView = normalize(viewPos - fragPos);

    // Ambient calculation
    vec3 ambient = light.ambientStrength * light.lightColor * meshes[pc.objectId].objectColor.rgb;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalizedNormal, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor * meshes[pc.objectId].objectColor.rgb;

    // Specular calculation
    vec3 reflectDir = reflect(-normalizedLightDir, normalizedNormal);
    float spec = pow(max(dot(normalizedView, reflectDir), 0.0), light.shininess);
    vec3 specular = light.specularStrength * spec * light.lightColor;

    // Final color
    vec3 finalColor = ambient + diffuse + specular;
    outColor = vec4(finalColor, 1.0);
}