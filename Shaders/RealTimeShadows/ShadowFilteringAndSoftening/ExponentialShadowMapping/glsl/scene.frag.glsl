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
    uint filterKernelSize;
    float esmExponent;
} pc;

float calculateShadow(vec3 normalWorldSpace, vec3 normalizedLightDir)
{
    vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos,1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    vec2 shadowUV = projCoords.xy;
    float currentDepth = projCoords.z;

    // Fixing shadowing problem outside the shadow map
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 0.0;
    }

    // Adjust kernel size (it should be odd number and at least 1)
    int kernelSize = int(pc.filterKernelSize);
    kernelSize = max(kernelSize, 1);
    if (kernelSize % 2 == 0) {
        kernelSize += 1;
    }

    // Apply Box Filtering
    vec2 texelSize = 1.0 / vec2(textureSize(uShadowMap, 0));
    float filteredShadowMapValue = 0.0;
    int sampleCount = 0;
    int radius = kernelSize / 2;
    for(int x = -radius; x <= radius; ++x)
    {
        for(int y = -radius; y <= radius; ++y)
        {
            vec2 offset = vec2(x, y) * texelSize;
            filteredShadowMapValue += texture(uShadowMap, shadowUV + offset).r;
            sampleCount++;
        }
    }
    filteredShadowMapValue /= float(sampleCount);

    // Calculate and return shadow
    float receiver = exp(pc.esmExponent * currentDepth);
    float visibility = clamp(filteredShadowMapValue / receiver, 0.0, 1.0);
    return 1.0 - visibility;
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