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
    float cameraFarPlane;
    uint objectId;
    uint enableShadowMapLod; // 1: Enabled, 0: Disabled
    uint shadowMapMaxLod;
} pc;

float calculateShadow(vec3 normalWorldSpace, vec3 normalizedLightDir)
{
    vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos,1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    vec2 shadowUV = projCoords.xy;
    float currentDepth = projCoords.z;

    // Calculate LOD for the current fragment
    float targetLod = 0.0;
    if (pc.enableShadowMapLod == 1) {
        vec3 viewPos = (pc.view * vec4(fragPos, 1.0)).xyz;
        float viewDepth = -viewPos.z;
        float t = clamp(viewDepth / pc.cameraFarPlane, 0.0, 1.0);
        targetLod = t * pc.shadowMapMaxLod;
    }

    // Calculate natural gradients
    vec2 dpdx = dFdx(shadowUV);
    vec2 dpdy = dFdy(shadowUV);

    // Fixing shadowing problem outside the shadow map
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 0.0;
    }

    // LOD implied by natural gradients
    // naturalLod = log2(max(|dpdx|, |dpdy|) * texSize)
    float texSize = float(textureSize(uShadowMap, 0).x);
    float rho = max(length(dpdx), length(dpdy)) * texSize;
    float naturalLod = log2(max(rho, 1e-6));

    // Preserve the anisotropy and set the size to target LOD
    float scale = exp2(targetLod - naturalLod);
    vec2 dpdxS = dpdx * scale;
    vec2 dpdyS = dpdy * scale;

    // Get moments from specific LOD
    vec2 moments = textureGrad(uShadowMap, shadowUV, dpdxS, dpdyS).rg;

    // Variance calculation
    float mean = moments.x; // E[z]
    float meanSq = moments.y;// E[z^2]
    float variance = meanSq - (mean * mean);
    variance = max(variance, 0.00002); // For light bleeding and precision fix

    float d = currentDepth - mean;

    // Chebyshev upper bound
    float p = variance / (variance + d * d);

    float visibility = (currentDepth <= mean) ? 1.0 : p;

    // Light bleed reducing
    float amount = 0.22;
    visibility = clamp((visibility - amount) / (1.0 - amount), 0.0, 1.0);
    return 1.0 - clamp(visibility, 0.0, 1.0);
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