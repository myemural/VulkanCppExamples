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

layout(location = 0) in vec3 fragPosView;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in mat3 fragTbnView;

layout(constant_id = 0) const uint LIGHT_COUNT = 0;

struct MeshMaterialData
{
    vec4 diffuseColor;
    float opacity;
    int diffuseMap;
    int normalMap;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(set = 0, binding = 2) uniform sampler2D uCombinedSamplers[];

struct PointLightData
{
    vec4 lightPosition;    // xyz = Light Position (View-Space)
    vec4 lightColor;       // rgb = Light Color
    vec4 pointLightParams; // x = Constant Factor, y = Linear Factor, z = Quadratic Factor
};

layout(std430, set = 0, binding = 3) readonly buffer PointLightBuffer
{
    PointLightData lights[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    uint objectId;
} pc;

vec3 calculateLight(PointLightData light, vec3 albedo, vec3 normalView, vec3 fragPosView, vec3 viewDir)
{
    // Constants
    const float specularStrength = 1.0;
    const float shininess = 64.0;

    float dist = length(light.lightPosition.xyz - fragPosView);
    float attenuation = 1.0 / (light.pointLightParams.x + light.pointLightParams.y * dist + light.pointLightParams.z * dist * dist);;

    vec3 lightDirView = normalize(light.lightPosition.xyz - fragPosView);

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalView, lightDirView), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * albedo;

    // Specular calculation
    vec3 halfDir = normalize(lightDirView + viewDir);
    float spec = pow(max(dot(normalView, halfDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * light.lightColor.rgb;

    // Final color (attenuation has less effect on specular)
    vec3 finalColor = (diffuse * attenuation) + (specular * sqrt(attenuation));
    return finalColor;
}

void main()
{
    // Constants
    const float ambientStrength = 0.02;

    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    vec3 albedo = meshMatInfo.diffuseColor.rgb;
    if (meshMatInfo.diffuseMap != -1) {
        vec4 diffuseTextureColor = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.diffuseMap)], fragUv);
        albedo = diffuseTextureColor.rgb;
    }

    // Normal map calculation
    vec3 normalViewSpace;
    if (meshMatInfo.normalMap != -1) {
        // Normal map sample (tangent space)
        vec3 normalTangent = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.normalMap)], fragUv).rgb;

        // Transform from [0,1] to [-1,1] range
        normalTangent = normalize(normalTangent * 2.0 - 1.0);

        // Tangent space to view space
        normalViewSpace = normalize(fragTbnView * normalTangent);
    }
    else
    {
        // Fallback: Geometric normal
        normalViewSpace = normalize(fragTbnView[2]);
    }

    // Lighting vectors in view space
    vec3 viewDir = normalize(-fragPosView);

    // Ambient calculation
    vec3 ambient = vec3(ambientStrength * albedo);

    vec3 resultColor = vec3(0.0);
    for (uint i = 0; i < LIGHT_COUNT; i++) {
        resultColor += calculateLight(lights[i], albedo, normalViewSpace, fragPosView, viewDir);
    }
    resultColor += ambient;

    outColor = vec4(resultColor, meshMatInfo.opacity);
}