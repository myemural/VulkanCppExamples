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
    vec4 albedoColor;
    float roughness;
    float metallic;
    float specular;
    float specularTint;
    float subsurface;
    float clearcoat;
    float clearcoatGloss;
    int albedoMap;
    int roughnessMap;
    int metallicMap;
    int normalMap;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightDirection;         // xyz = Light Direction
    vec4 lightColorAndIntensity; // rgb = Light Color, a = Light Intensity
} light;

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

layout(set = 0, binding = 4) uniform samplerCube irradianceMap;

layout(set = 0, binding = 5) uniform samplerCube prefilterMap;

layout(set = 0, binding = 6) uniform sampler2D brdfLut;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    uint prefilterRoughnessMaxMipCount;
    uint isIblEnabled; // 0: IBL Disabled, 1: IBL Enabled
} pc;

const float PI = 3.14159265;

// Modified Schlick Fresnel auxiliary function to be used in all steps
// schlickFresnel(0) = 0: Normal incidence
// schlickFresnel(1) = 1: Grazing angle
float schlickFresnel(float u)
{
    float m = clamp(1.0 - u, 0.0, 1.0);
    return pow(m, 5);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0)
    * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Computes a tint/chromaticity color by removing the luminance component from the base color
// Reference: https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf
vec3 computeAlbedoTint(vec3 albedo)
{
    float luminance = 0.3 * albedo.r + 0.6 * albedo.g + 0.1 * albedo.b;
    return luminance > 0.0 ? albedo / luminance : vec3(1.0);
}

// Disney's base diffuse model
// Reference: https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf
float calculateDiffuseFresnel(float NdotL, float NdotV, float LdotH, float roughness)
{
    float FL = schlickFresnel(NdotL);
    float FV = schlickFresnel(NdotV);
    float Fd90 = 0.5 + 2.0 * LdotH * LdotH * roughness;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);
    return Fd;
}

// Subsurface approximation based on Hanrahan-Krueger brdf approximation
// Reference: https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf
float calculateSubsurface(float NdotL, float NdotV, float LdotH, float roughness)
{
    float FL = schlickFresnel(NdotL);
    float FV = schlickFresnel(NdotV);

    // FSS90 smooths out retroreflection as roughness increases
    float Fss90 = LdotH * LdotH * roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);

    // 1 / (NdotL + NdotV): Simulates light transmission at the surface edge
    float ss = 1.25 * (Fss * (1.0 / max(NdotL + NdotV, 0.0001) - 0.5) + 0.5);
    return ss;
}

// GTR2 (Generalized Trowbridge-Reitz, Disney expression of GGX without anisotropy
float calculateGTR2(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
    return a2 / (PI * t * t);
}

// Disney's Smith GGX without anisotropy
float calculateSmithGGX(float NdotV, float alphaG)
{
    float a = alphaG * alphaG;
    float b = NdotV * NdotV;
    return 1.0 / (NdotV + sqrt(a + b - a * b));
}

// GTR1 Berry Distribuiton, for clear coat calculation
// Reference: https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf
float calculateGTR1(float NdotH, float a)
{
    if (a >= 1.0)
    {
        return 1.0 / PI;
    }

    float a2 = a * a;
    float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
    return (a2 - 1.0) / (PI * log(a2) * t);
}

void main()
{
    // Get mesh info
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];

    // Normal map calculation
    vec3 normalWorldSpace;
    if (meshInfo.normalMap != -1) {
        // Normal map sample (tangent space)
        vec3 normalTangent = texture(uCombinedSamplers[nonuniformEXT(meshInfo.normalMap)], fragUv).rgb;

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

    // Normalizing fundamental vectors
    vec3 normNormal = normalWorldSpace;
    vec3 normLightDir = normalize(-light.lightDirection.xyz);
    vec3 normViewDir = normalize(pc.cameraPosition.xyz - fragPos);
    vec3 normHalfDir = normalize(normLightDir + normViewDir);
    vec3 reflectDir = reflect(-normViewDir, normNormal);

    // Calculate radiance of the light
    vec3 radiance = light.lightColorAndIntensity.rgb * light.lightColorAndIntensity.a;

    vec3 albedo = meshInfo.albedoColor.rgb;
    if (meshInfo.albedoMap != -1) {
        albedo = texture(uCombinedSamplers[nonuniformEXT(meshInfo.albedoMap)], fragUv).rgb;
    }

    float roughness = meshInfo.roughness;
    if (meshInfo.roughnessMap != -1) {
        roughness = texture(uCombinedSamplers[nonuniformEXT(meshInfo.roughnessMap)], fragUv).r;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    float metallic = meshInfo.metallic;
    if (meshInfo.metallicMap != -1) {
        metallic = texture(uCombinedSamplers[nonuniformEXT(meshInfo.metallicMap)], fragUv).r;
    }

    // Calculate dot products
    float NdotL = max(dot(normNormal, normLightDir), 0.0);
    float NdotV = max(dot(normNormal, normViewDir), 0.0);
    float LdotH = max(dot(normLightDir, normHalfDir), 0.0);
    float NdotH = max(dot(normNormal, normHalfDir), 0.0);

    // Precompute some color values that used in the BRDF formulas
    vec3 albedoTintColor = computeAlbedoTint(albedo);
    vec3 specularColorF0 = mix(meshInfo.specular * 0.08 * mix(vec3(1.0), albedoTintColor, meshInfo.specularTint), albedo, metallic);

    // Calculate diffuse Fresnel and subsurface approximation
    float Fd = calculateDiffuseFresnel(NdotL, NdotV, LdotH, roughness);
    float subsurface = calculateSubsurface(NdotL, NdotV, LdotH, roughness);

    // Calculate specular part
    float FH = schlickFresnel(LdotH);
    vec3 Fs = mix(specularColorF0, vec3(1.0), FH);
    float Ds = calculateGTR2(NdotH, roughness);
    float sqrRoughness = roughness * roughness;
    float Gs = calculateSmithGGX(NdotL, sqrRoughness) * calculateSmithGGX(NdotV, sqrRoughness);
    vec3 specularPart = Fs * Ds * Gs;

    // Clear coat calculation
    float Fc = mix(0.04, 1.0, FH);
    float Dc = calculateGTR1(NdotH, mix(0.1, 0.001, meshInfo.clearcoatGloss));
    float Gc = calculateSmithGGX(NdotL, 0.25) * calculateSmithGGX(NdotV, 0.25);
    vec3 clearcoatLobe = vec3(0.25 * meshInfo.clearcoat * Fc * Dc * Gc);

    // Calculate disney BRDF for fundamental diffuse and specular
    vec3 disneyBrdf = ((1/PI) * mix(Fd, subsurface, meshInfo.subsurface) * albedo) * (1-metallic) + specularPart + clearcoatLobe;

    // Direct lighting
    vec3 directLighting = disneyBrdf * radiance * NdotL;

    // Diffuse irradiance IBL
    vec3 ambientKS = fresnelSchlickRoughness(NdotV, specularColorF0, roughness);
    vec3 ambientKD = (1.0 - ambientKS) * (1.0 - metallic);
    vec3 irradiance = texture(irradianceMap, normNormal).rgb;
    vec3 diffuseIBL = irradiance * albedo * ambientKD;

    // Specular IBL
    const float prefilterMaxLod = float(pc.prefilterRoughnessMaxMipCount) - 1.0;
    vec3 prefilteredColor = textureLod(prefilterMap, reflectDir, roughness * prefilterMaxLod).rgb;
    vec2 brdfLutSample = texture(brdfLut, vec2(NdotV, roughness)).rg;
    vec3 specularIBL = prefilteredColor * (ambientKS * brdfLutSample.x + brdfLutSample.y);

    // IBL for clear coat specular
    float ccRoughnessIBL = clamp(1.0 - meshInfo.clearcoatGloss, 0.04, 1.0);
    vec3 ccPrefilteredColor = textureLod(prefilterMap, reflectDir, ccRoughnessIBL * prefilterMaxLod).rgb;
    vec2 ccBrdf = texture(brdfLut, vec2(NdotV, ccRoughnessIBL)).rg;
    float ccFresnel = 0.04 + 0.96 * schlickFresnel(NdotV); // F0 = 0.04
    vec3 clearcoatIBL = meshInfo.clearcoat * ccPrefilteredColor * (ccFresnel * ccBrdf.x + ccBrdf.y);

    // Total ambient calculation
    vec3 ambient = diffuseIBL + specularIBL + clearcoatIBL;

    // Final outgoing light
    vec3 Lo = directLighting;

    if (pc.isIblEnabled == 1) {
        Lo += ambient;
    }

    // Apply simple reinhard tone mapping for HDR to LDR
    vec3 ldrColor = Lo / (Lo + vec3(1.0));
    outColor = vec4(ldrColor, 1.0);
}