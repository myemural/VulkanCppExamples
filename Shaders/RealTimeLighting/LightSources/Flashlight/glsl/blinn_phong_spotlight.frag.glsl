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

struct MeshMaterialData
{
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightPosition;    // xyz = Light Position
    vec4 lightDirection;   // xyz = Light Direction (normalized)
    vec4 lightColor;       // rgb = Light Color
    vec4 spotlightParams;  // x = cos(innerCutoffAngle), y = cos(outerCutoffAngle)
} light;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

vec3 calculateLight(vec3 normalizedNormal, vec3 fragmentPosition)
{
    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(light.lightPosition.xyz - fragmentPosition);

    // Normalizing view direction (camera position)
    vec3 normalizedView = normalize(pc.cameraPosition.xyz - fragPos);

    // Ambient calculation
    vec3 ambient = meshMatInfo.ambientStrength * meshMatInfo.diffuseColor.rgb;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalizedNormal, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * meshMatInfo.diffuseColor.rgb;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalizedNormal, halfDir), 0.0), meshMatInfo.shininess);
    vec3 specular = meshMatInfo.specularStrength * spec * light.lightColor.rgb * meshMatInfo.specularColor.rgb;

    // Spotlight contribution calculation (hard-cutoff)
    vec3 spotDir = normalize(-light.lightDirection.xyz);
    float theta = dot(normalizedLightDir, spotDir);

    float innerCutoff = light.spotlightParams.x;
    float outerCutoff = light.spotlightParams.y;
    float spotFactor = smoothstep(outerCutoff, innerCutoff, theta);

    // Final color
    vec3 finalColor = ambient + spotFactor * (diffuse + specular);
    return finalColor;
}

void main()
{
    // Normalizing normal
    vec3 normalizedNormal = normalize(fragNormal);

    vec3 resultColor = vec3(0.0);
    resultColor += calculateLight(normalizedNormal, fragPos);
    outColor = vec4(resultColor, 1.0);
}