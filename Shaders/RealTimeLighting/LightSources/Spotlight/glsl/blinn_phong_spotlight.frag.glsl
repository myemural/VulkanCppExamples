#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define MAX_LIGHTS 2

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

struct LightUBO
{
    vec4 lightPosition;    // xyz = Light Position
    vec4 lightDirection;   // xyz = Light Direction (normalized)
    vec4 lightColor;       // rgb = Light Color
    vec4 ambientParams;    // x = Ambient Strength
    vec4 specularParams;   // x = Specular Strength, y = Shininess
    vec4 spotlightParams;  // x = cos(cutoffAngle)
};

layout(std140, set = 0, binding = 1) uniform LightBlock
{
    LightUBO lights[MAX_LIGHTS];
} lightBlock;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    uint objectId;
} pc;

vec3 calculateLight(LightUBO light, vec3 normalizedNormal, vec3 fragmentPosition)
{
    // Normalizing light direction
    vec3 normalizedLightDir = normalize(light.lightPosition.xyz - fragmentPosition);

    // Normalizing view direction (camera position)
    mat4 inverseView = inverse(pc.view);
    vec3 viewPos = vec3(inverseView[3]);
    vec3 normalizedView = normalize(viewPos - fragmentPosition);

    // Ambient calculation
    vec3 ambient = light.ambientParams.x * light.lightColor.rgb * meshes[pc.objectId].objectColor.rgb;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalizedNormal, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * meshes[pc.objectId].objectColor.rgb;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalizedNormal, halfDir), 0.0), light.specularParams.y);
    vec3 specular = light.specularParams.x * spec * light.lightColor.rgb;

    // Spotlight contribution calculation (hard-cutoff)
    vec3 spotDir = normalize(-light.lightDirection.xyz);
    float theta = dot(normalizedLightDir, spotDir);
    float spotFactor = step(light.spotlightParams.x, theta);

    // Final color
    vec3 finalColor = ambient + spotFactor * (diffuse + specular);
    return finalColor;
}

void main()
{
    // Normalizing normal
    vec3 normalizedNormal = normalize(fragNormal);

    vec3 resultColor = vec3(0.0);
    for (int i = 0; i < MAX_LIGHTS; i++) {
        resultColor += calculateLight(lightBlock.lights[i], normalizedNormal, fragPos);
    }

    outColor = vec4(resultColor, 1.0);
}