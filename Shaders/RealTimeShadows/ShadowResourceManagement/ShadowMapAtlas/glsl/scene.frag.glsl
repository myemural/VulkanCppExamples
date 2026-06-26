#version 450
#extension GL_EXT_nonuniform_qualifier : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define NUM_OF_LIGHTS 4

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in mat3 fragTBN;

struct MeshMaterialData
{
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float uvScale;
    int diffuseMap;
    int normalMap;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

struct LightData
{
    mat4 lightSpaceMatrix; // Light-space matrix
    vec4 lightPosition;    // xyz = Light Position (for spotlight only)
    vec4 lightDirection;   // xyz = Light Direction
    vec4 lightColor;       // xyz = Light Color
    vec4 lightParams;      // x = Light Type (0: Directional, 1: Spot), y = cos(innerCutoffAngle) for spotlight,
                           // z = cos(outerCutoffAngle) for spotlight
    vec4 shadowMapAtlasRect;
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    LightData lightData[NUM_OF_LIGHTS];
} light;

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

layout(set = 0, binding = 4) uniform sampler2D uShadowMapAtlas;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

float calculateShadow(mat4 lightSpaceMatrix, vec4 atlasRect, vec3 normalWorldSpace, vec3 normalizedLightDir)
{
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos,1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    vec2 shadowUV = projCoords.xy;
    float currentDepth = projCoords.z;

    // Fixing shadowing problem outside the shadow map
    if (currentDepth > 1.0) {
        return 0.0;
    }

    vec2 atlasUV = atlasRect.xy + projCoords.xy * atlasRect.zw;

    float closestDepth = texture(uShadowMapAtlas, atlasUV).r;
    closestDepth = closestDepth * 0.5 + 0.5;

    // Fixing shadow acne
    float bias = max(0.003 * (1.0 - dot(normalWorldSpace, normalizedLightDir)), 0.0001);

    return (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
}

vec3 calculateSpotLight(LightData lightData, vec3 normalWorldSpace, vec3 fragmentPosition)
{
    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(lightData.lightPosition.xyz - fragmentPosition);

    // Normalizing view direction (camera position)
    vec3 normalizedView = normalize(pc.cameraPosition.xyz - fragPos);

    // Diffuse color calculation
    const vec2 scaledUv = fragUv * meshMatInfo.uvScale;
    vec3 diffuseColor = meshMatInfo.diffuseColor.rgb;
    if (meshMatInfo.diffuseMap != -1) {
        vec4 diffuseTextureColor = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.diffuseMap)], scaledUv);
        diffuseColor = diffuseTextureColor.rgb;
    }

    // Ambient calculation
    vec3 ambient = meshMatInfo.ambientStrength * diffuseColor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalWorldSpace, normalizedLightDir), 0.0);
    vec3 diffuse = diff * lightData.lightColor.rgb * diffuseColor;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalWorldSpace, halfDir), 0.0), meshMatInfo.shininess);
    vec3 specular = meshMatInfo.specularStrength * spec * lightData.lightColor.rgb * meshMatInfo.specularColor.rgb;

    // Spotlight contribution calculation (soft-cutoff)
    vec3 spotDir = normalize(-lightData.lightDirection.xyz);
    float theta = dot(normalizedLightDir, spotDir);

    float innerCutoff = lightData.lightParams.y;
    float outerCutoff = lightData.lightParams.z;
    float spotFactor = smoothstep(outerCutoff, innerCutoff, theta);

    // Final color
    float shadow = calculateShadow(lightData.lightSpaceMatrix, lightData.shadowMapAtlasRect, normalWorldSpace, normalizedLightDir);
    vec3 finalColor = ambient + (1.0 - shadow) * spotFactor * (diffuse + specular);
    return finalColor;
}

vec3 calculateDirectionalLight(LightData lightData, vec3 normalWorldSpace, vec3 fragmentPosition)
{
    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(-lightData.lightDirection.xyz);

    // Normalizing view direction (camera position)
    vec3 normalizedView = normalize(pc.cameraPosition.xyz - fragPos);

    // Diffuse color calculation
    const vec2 scaledUv = fragUv * meshMatInfo.uvScale;
    vec3 diffuseColor = meshMatInfo.diffuseColor.rgb;
    if (meshMatInfo.diffuseMap != -1) {
        vec4 diffuseTextureColor = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.diffuseMap)], scaledUv);
        diffuseColor = diffuseTextureColor.rgb;
    }

    // Ambient calculation
    vec3 ambient = meshMatInfo.ambientStrength * diffuseColor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalWorldSpace, normalizedLightDir), 0.0);
    vec3 diffuse = diff * lightData.lightColor.rgb * diffuseColor;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalWorldSpace, halfDir), 0.0), meshMatInfo.shininess);
    vec3 specular = meshMatInfo.specularStrength * spec * lightData.lightColor.rgb * meshMatInfo.specularColor.rgb;

    // Final color
    float shadow = calculateShadow(lightData.lightSpaceMatrix, lightData.shadowMapAtlasRect, normalWorldSpace, normalizedLightDir);
    vec3 finalColor = ambient + (1.0 - shadow) * (diffuse + specular);
    return finalColor;
}

void main()
{
    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    // Calculate scaled UV
    const vec2 scaledUv = fragUv * meshMatInfo.uvScale;

    // Normal map calculation
    vec3 normalWorldSpace;
    if (meshMatInfo.normalMap != -1) {
        // Normal map sample (tangent space)
        vec3 normalTangent = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.normalMap)], scaledUv).rgb;

        // Transform from [0,1] to [-1,1] range
        normalTangent = normalize(normalTangent * 2.0 - 1.0);

        // Tangent space to world space
        normalWorldSpace = normalize(fragTBN * normalTangent);
    }
    else
    {
        // Fallback: Geometric normal
        normalWorldSpace = normalize(fragTBN[2]);
    }

    // Final color with shadow
    vec3 resultColor = vec3(0.0);

    for (int i = 0; i < NUM_OF_LIGHTS; ++i)
    {
        if (uint(light.lightData[i].lightParams.x) == 0) {
            resultColor += calculateDirectionalLight(light.lightData[i], normalWorldSpace, fragPos);
        } else {
            resultColor += calculateSpotLight(light.lightData[i], normalWorldSpace, fragPos);
        }
    }

    outColor = vec4(resultColor, 1.0);
}