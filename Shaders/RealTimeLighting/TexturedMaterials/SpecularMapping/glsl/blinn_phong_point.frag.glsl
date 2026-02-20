#version 450
#extension GL_EXT_nonuniform_qualifier : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;

struct MeshMaterialData
{
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    int diffuseMap;
    int specularMap;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightPosition;    // xyz = Light Position
    vec4 lightColor;       // rgb = Light Color
    vec4 pointLightParams; // x = Constant Factor, y = Linear Factor, z = Quadratic Factor
} light;

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

void main()
{
    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    vec3 diffuseColor = meshMatInfo.diffuseColor.rgb;
    if (meshMatInfo.diffuseMap != -1) {
        vec4 diffuseTextureColor = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.diffuseMap)], fragUv);
        diffuseColor = diffuseTextureColor.rgb;
    }

    vec3 specularColor = meshMatInfo.specularColor.rgb;
    if (meshMatInfo.specularMap != -1) {
        vec4 specularTextureColor = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.specularMap)], fragUv);
        specularColor = specularTextureColor.rgb;
    }

    // Normalizing normal
    vec3 normalizedNormal = normalize(fragNormal);

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(light.lightPosition.xyz - fragPos);

    // Normalizing view direction (camera position)
    vec3 normalizedView = normalize(pc.cameraPosition.xyz - fragPos);

    // Ambient calculation
    vec3 ambient = meshMatInfo.ambientStrength * diffuseColor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalizedNormal, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * diffuseColor;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalizedNormal, halfDir), 0.0), meshMatInfo.shininess);
    vec3 specular = meshMatInfo.specularStrength * spec * light.lightColor.rgb * specularColor;

    // Attenuation calculation
    float distance = length(light.lightPosition.xyz - fragPos);
    float attenuation = 1.0 / (light.pointLightParams.x + light.pointLightParams.y * distance + light.pointLightParams.z * (distance * distance));

    // Apply attenuation to all components
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // Final color
    vec3 finalColor = ambient + diffuse + specular;
    outColor = vec4(finalColor, 1.0);
}