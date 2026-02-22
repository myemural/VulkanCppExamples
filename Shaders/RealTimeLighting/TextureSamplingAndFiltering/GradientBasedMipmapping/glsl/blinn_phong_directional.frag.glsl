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
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightDirection; // xyz = Light Direction
    vec4 lightColor;     // rgb = Light Color
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

    // Normalizing normal
    vec3 normalizedNormal = normalize(fragNormal);

    vec3 diffuseColor = meshMatInfo.diffuseColor.rgb;
    if (meshMatInfo.diffuseMap != -1) {
        // Calculate mipmap LOD level
        float distanceToCamera = length(pc.cameraPosition.xyz - fragPos);
        float lodFactor = clamp(distanceToCamera / 50.0, 0.0, 1.0);

        // Scale texture to see anisotropic filtering effect well
        vec2 uv = fragUv * 4.0;

        // UV derivatives
        vec2 dx = dFdx(fragUv) * 4.0;
        vec2 dy = dFdy(fragUv) * 4.0;

        // Scale gradient
        float lodScale = mix(1.0, 16.0, lodFactor);
        dx *= lodScale;
        dy *= lodScale;

        vec4 diffuseTextureColor = textureGrad(uCombinedSamplers[nonuniformEXT(meshMatInfo.diffuseMap)], uv, dx, dy);
        // Mix the texture color with red depending on the distance
        diffuseColor = mix(diffuseTextureColor.rgb, vec3(1.0, 0.0, 0.0), lodFactor);
    }

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(-light.lightDirection.xyz);

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
    vec3 specular = meshMatInfo.specularStrength * spec * light.lightColor.rgb * meshMatInfo.specularColor.rgb;

    // Final color
    vec3 finalColor = ambient + diffuse + specular;
    outColor = vec4(finalColor, 1.0);
}