#version 450
#extension GL_EXT_nonuniform_qualifier : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define POISSON_DISK_SIZE 16

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

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightDirection;   // xyz = Light Direction
    vec4 lightColor;       // rgb = Light Color
    mat4 lightSpaceMatrix; // Light-space matrix
} light;

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

layout(set = 0, binding = 4) uniform sampler2D uShadowMap;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    float lightOrthoSize;
} pc;

// Generated poisson disk
/// TODO: Rotated poisson logic can be added later.
vec2 poissonDisk[POISSON_DISK_SIZE] = vec2[](
    vec2(-0.94201624, -0.39906216),
    vec2(0.94558609, -0.76890725),
    vec2(-0.09418410, -0.92938870),
    vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845),
    vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554),
    vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507),
    vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367),
    vec2(0.14383161, -0.14100790)
);

float findBlocker(vec2 uv, float currentDepth, float bias, vec2 texelSize)
{
    float avg = 0.0;
    int count = 0;

    for (int i = 0; i < POISSON_DISK_SIZE; i++)
    {
        vec2 offset = poissonDisk[i] * texelSize;
        float depth = texture(uShadowMap, uv + offset).r;

        if (depth < (currentDepth - bias)) {
            avg += depth;
            count++;
        }
    }

    return (count != 0) ? avg / float(count) : -1;
}

float computePenumbra(float receiverDepth, float blockerDepth, float lightSize)
{
    return (receiverDepth - blockerDepth) * lightSize / blockerDepth;
}

float pcfFilter(vec2 uv, float currentDepth, float bias, vec2 texelSize, float filterRadius)
{
    float shadow = 0.0;
    for (int i = 0; i < POISSON_DISK_SIZE; i++)
    {
        vec2 offset = poissonDisk[i] * texelSize * filterRadius;
        float depth = texture(uShadowMap, uv + offset).r;

        shadow += (currentDepth - bias) > depth ? 1.0 : 0.0;
    }

    return shadow / float(POISSON_DISK_SIZE);
}

float calculateShadow(vec3 normalWorldSpace, vec3 normalizedLightDir)
{
    vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos,1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    vec2 shadowUV = projCoords.xy;
    float currentDepth = projCoords.z;

    // Fixing shadowing problem outside the shadow map
    if (currentDepth > 1.0) {
        return 0.0;
    }

    float closestDepth = texture(uShadowMap, shadowUV).r;

    // Fixing shadow acne
    float bias = max(0.003 * (1.0 - dot(normalWorldSpace, normalizedLightDir)), 0.0001);

    // Calculate texel size
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);

    // Step 1: Blocker search
    float averageBlockerDepth = findBlocker(shadowUV, currentDepth, bias, texelSize);

    // If no blocker, return with the no shadow
    if (averageBlockerDepth == -1.0) {
        return 0.0;
    }

    // Step 2: Penumbra computation
    float lightSize = pc.lightOrthoSize / textureSize(uShadowMap, 0).x;
    float penumbra = computePenumbra(currentDepth, averageBlockerDepth, lightSize);

    // Step 3: Adaptive PCF
    float filterRadius = penumbra * textureSize(uShadowMap, 0).x * 0.1;
    float shadow = pcfFilter(shadowUV, currentDepth, bias, texelSize, filterRadius);

    return shadow;
}

void main()
{
    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    // Calculate scaled UV
    const vec2 scaledUv = fragUv * meshMatInfo.uvScale;

    vec3 diffuseColor = meshMatInfo.diffuseColor.rgb;
    if (meshMatInfo.diffuseMap != -1) {
        vec4 diffuseTextureColor = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.diffuseMap)], scaledUv);
        diffuseColor = diffuseTextureColor.rgb;
    }

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

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(-light.lightDirection.xyz);

    // Normalizing view direction (camera position)
    vec3 normalizedView = normalize(pc.cameraPosition.xyz - fragPos);

    // Ambient calculation
    vec3 ambient = meshMatInfo.ambientStrength * diffuseColor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalWorldSpace, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * diffuseColor;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalWorldSpace, halfDir), 0.0), meshMatInfo.shininess);
    vec3 specular = meshMatInfo.specularStrength * spec * light.lightColor.rgb * meshMatInfo.specularColor.rgb;

    // Final color with shadow
    float shadow = calculateShadow(normalWorldSpace, normalizedLightDir);
    vec3 finalColor = ambient + (1.0 - shadow) * (diffuse + specular);
    outColor = vec4(finalColor, 1.0);
}