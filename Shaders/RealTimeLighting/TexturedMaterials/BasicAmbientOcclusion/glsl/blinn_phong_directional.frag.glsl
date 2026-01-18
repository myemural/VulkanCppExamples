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

struct MeshData {
    mat4 model;
    mat4 normalMatrix;
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
    int aoMap;
    int heightMap;
};

layout(std430, binding = 0) readonly buffer MeshDataBuffer {
    MeshData meshes[];
};

layout(std140, set = 0, binding = 1) uniform LightUBO
{
    vec4 lightDirection;    // xyz = Light Direction
    vec4 lightColor;        // rgb = Light Color
    vec4 lightAmbientColor; // rgb = Light Color
} light;

layout(set = 0, binding = 2) uniform sampler2D uCombinedSamplers[];

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

    vec3 diffuseColor = meshInfo.diffuseColor.rgb;
    if (meshInfo.diffuseMap != -1) {
        vec4 diffuseTextureColor = texture(uCombinedSamplers[nonuniformEXT(meshInfo.diffuseMap)], fragUv);
        diffuseColor = diffuseTextureColor.rgb;
    }

    float ao = 1.0f;
    if (meshInfo.aoMap != -1) {
        ao = texture(uCombinedSamplers[nonuniformEXT(meshInfo.aoMap)], fragUv).r;
    }

    // Normalizing normal
    vec3 normalizedNormal = normalize(fragNormal);

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(-light.lightDirection.xyz);

    // Normalizing view direction (camera position)
    vec3 normalizedView = normalize(pc.cameraPosition.xyz - fragPos);

    // Ambient calculation
    vec3 ambient = meshInfo.ambientStrength * ao * light.lightAmbientColor.rgb;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalizedNormal, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * diffuseColor;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalizedNormal, halfDir), 0.0), meshInfo.shininess);
    vec3 specular = meshInfo.specularStrength * spec * light.lightColor.rgb * meshInfo.specularColor.rgb;

    // Final color
    vec3 finalColor = ambient + diffuse + specular;
    outColor = vec4(finalColor, meshInfo.opacity);
}