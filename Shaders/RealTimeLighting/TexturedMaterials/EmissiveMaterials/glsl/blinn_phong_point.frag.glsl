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
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;

layout(constant_id = 0) const uint TEXTURE_COUNT = 1;

struct MeshData {
    mat4 model;
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
};

layout(std430, binding = 0) readonly buffer MeshDataBuffer {
    MeshData meshes[];
};

layout(std140, set = 0, binding = 1) uniform LightUBO
{
    vec4 lightPosition;    // xyz = Light Position
    vec4 lightColor;       // rgb = Light Color
    vec4 pointLightParams; // x = Constant Factor, y = Linear Factor, z = Quadratic Factor
} light;

layout(set = 0, binding = 2) uniform sampler2D uCombinedSamplers[TEXTURE_COUNT];

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    uint objectId;
} pc;

void main()
{
    // Get mesh info
    const MeshData meshInfo = meshes[pc.objectId];

    vec3 diffuseColor = meshInfo.diffuseColor.rgb;
    if (meshInfo.diffuseMap != -1) {
        vec4 diffuseTextureColor = texture(uCombinedSamplers[meshInfo.diffuseMap], fragUv);
        diffuseColor = diffuseTextureColor.rgb;
    }

    // Normalizing normal
    vec3 normalizedNormal = normalize(fragNormal);

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(light.lightPosition.xyz - fragPos);

    // Normalizing view direction (camera position)
    mat4 inverseView = inverse(pc.view);
    vec3 viewPos = vec3(inverseView[3]);
    vec3 normalizedView = normalize(viewPos - fragPos);

    // Ambient calculation
    vec3 ambient = meshInfo.ambientStrength * diffuseColor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalizedNormal, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * diffuseColor;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalizedNormal, halfDir), 0.0), meshInfo.shininess);
    vec3 specular = meshInfo.specularStrength * spec * light.lightColor.rgb * meshInfo.specularColor.rgb;

    // Attenuation calculation
    float distance = length(light.lightPosition.xyz - fragPos);
    float attenuation = 1.0 / (light.pointLightParams.x + light.pointLightParams.y * distance + light.pointLightParams.z * (distance * distance));

    // Apply attenuation to all components
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 emissiveColor = vec3(0.0);
    if (meshInfo.emissiveMap != -1) {
        emissiveColor = texture(uCombinedSamplers[meshInfo.emissiveMap], fragUv).rgb;
    }

    // Final color
    vec3 finalColor = ambient + diffuse + specular + emissiveColor;
    outColor = vec4(finalColor, meshInfo.opacity);
}