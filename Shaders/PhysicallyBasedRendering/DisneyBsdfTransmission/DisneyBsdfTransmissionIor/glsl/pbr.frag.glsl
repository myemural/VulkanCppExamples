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
    vec4 transmittanceColor; // rgb = Volumetric transmittance color, a = Unused
    float roughness;
    float metallic;
    float specularTint;
    float anisotropic;
    float sheen;
    float sheenTint;
    float clearcoat;
    float clearcoatGloss;
    float ior;
    float specTrans; // Specular Transmission
    float atDistance; // Distance at which transmittanceColor is reached
    float thickness; // Estimated path length of the refracted ray
    float uvScale;
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

layout(set = 0, binding = 7) uniform sampler2D sceneColorMap;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    uint prefilterRoughnessMaxMipCount;
    uint isIblEnabled; // 0: IBL Disabled, 1: IBL Enabled
    uint isToneMapEnabled;
} pc;

const float PI = 3.14159265;

// Relative IOR of the surrounding medium (air)
const float kAmbientIor = 1.0;

float sqr(float x) { return x * x; }

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

// Exact (unpolarized) Fresnel reflectance of a smooth dielectric interface
float fresnelDielectric(float cosThetaI, float eta)
{
    cosThetaI = clamp(cosThetaI, 0.0, 1.0);

    // Snell's law in squared form: sin(thetaT) = eta * sin(thetaI)
    float sin2ThetaI = 1.0 - cosThetaI * cosThetaI;
    float sin2ThetaT = eta * eta * sin2ThetaI;

    // Total internal reflection: Nothing is transmitted
    if (sin2ThetaT >= 1.0) {
        return 1.0;
    }

    float cosThetaT = sqrt(1.0 - sin2ThetaT);

    // Perpendicular and parallel polarized reflectance
    float Rs = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);
    float Rp = (cosThetaI - eta * cosThetaT) / (cosThetaI + eta * cosThetaT);

    return clamp(0.5 * (Rs * Rs + Rp * Rp), 0.0, 1.0);
}

// Computes a tint/chromaticity color by removing the luminance component from the base color
// Reference: https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf
vec3 computeAlbedoTint(vec3 albedo)
{
    float luminance = 0.3 * albedo.r + 0.6 * albedo.g + 0.1 * albedo.b;
    return luminance > 0.0 ? albedo / luminance : vec3(1.0);
}

// Normal incidence reflectance of a dielectric interface, derived from the index of the refraction
// ior = 1.5 (common glass) : F0 = 0.04
// ior = 2.5 (diamond) : F0 = 0.18
float computeDielectricF0(float ior)
{
    return sqr((ior - kAmbientIor) / (ior + kAmbientIor));
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

// GTR2 anisotropic version
// It takes different roughness parameters (ax, ay) for the X (tangent) and Y (bitangent) directions
// Reference: https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf
float calculateGTR2Aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
{
    float denom = sqr(HdotX / ax) + sqr(HdotY / ay) + NdotH * NdotH;
    return 1.0 / (PI * ax * ay * denom * denom);
}

// Smith GGX anisotropic version
// Reference: https://github.com/wdas/brdf/blob/main/src/brdfs/disney.brdf
float calculateSmithGGXAniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
    return 1.0 / (NdotV + sqrt(sqr(VdotX * ax) + sqr(VdotY * ay) + sqr(NdotV)));
}

// Screen-space refraction
vec3 sampleTransmittedRadiance(vec3 exitPosition, float roughness)
{
    // Screen-space refraction stays stable only while the lookup remains in the coarse but still screen-sized mips
    const float maxTransmissionLod = 6.0;

    vec4 clipPosition = pc.proj * pc.view * vec4(exitPosition, 1.0);

    // Behind the near plane, nothing sensible can be sampled
    if (clipPosition.w <= 0.0) {
        return vec3(0.0);
    }

    vec2 refractionUv = (clipPosition.xy / clipPosition.w) * 0.5 + 0.5;
    refractionUv = clamp(refractionUv, vec2(0.0), vec2(1.0));

    float maxLod = min(float(max(textureQueryLevels(sceneColorMap) - 1, 0)), maxTransmissionLod);
    return textureLod(sceneColorMap, refractionUv, roughness * maxLod).rgb;
}

void main()
{
    // Get mesh info
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];

    // Calculate scaled UV
    const vec2 scaledUv = fragUv * meshInfo.uvScale;

    // Double-sided surface handling
    const bool isBackFace = gl_FrontFacing;

    vec3 tangentWorldSpace = normalize(fragTBN[0]);
    vec3 bitangentWorldSpace = normalize(fragTBN[1]);
    vec3 geomNormalWorldSpace = normalize(fragTBN[2]);

    if (isBackFace) {
        bitangentWorldSpace = -bitangentWorldSpace;
        geomNormalWorldSpace = -geomNormalWorldSpace;
    }

    // Normal map calculation
    vec3 normalWorldSpace;
    if (meshInfo.normalMap != -1) {
        const mat3 shadingTBN = mat3(tangentWorldSpace, bitangentWorldSpace, geomNormalWorldSpace);

        vec3 normalTangent = texture(uCombinedSamplers[nonuniformEXT(meshInfo.normalMap)], scaledUv).rgb;
        normalTangent = normalize(normalTangent * 2.0 - 1.0);
        normalWorldSpace = normalize(shadingTBN * normalTangent);
    }
    else
    {
        // Fallback: Geometric normal
        normalWorldSpace = geomNormalWorldSpace;
    }

    // Normalizing view direction vector
    vec3 normViewDir = normalize(pc.cameraPosition.xyz - fragPos);

    // Normalizing fundamental vectors
    vec3 normNormal = normalWorldSpace;
    vec3 normLightDir = normalize(-light.lightDirection.xyz);
    vec3 halfDirRaw = normLightDir + normViewDir;
    float halfDirLength = length(halfDirRaw);
    bool hasValidHalfDir = halfDirLength > 1e-5;
    vec3 normHalfDir = hasValidHalfDir ? halfDirRaw / halfDirLength : normNormal;
    vec3 reflectDir = reflect(-normViewDir, normNormal);

    // Calculate radiance of the light
    vec3 radiance = light.lightColorAndIntensity.rgb * light.lightColorAndIntensity.a;

    vec3 albedo = meshInfo.albedoColor.rgb;
    if (meshInfo.albedoMap != -1) {
        albedo = texture(uCombinedSamplers[nonuniformEXT(meshInfo.albedoMap)], scaledUv).rgb;
    }

    float roughness = meshInfo.roughness;
    if (meshInfo.roughnessMap != -1) {
        roughness = texture(uCombinedSamplers[nonuniformEXT(meshInfo.roughnessMap)], scaledUv).r;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    float metallic = meshInfo.metallic;
    if (meshInfo.metallicMap != -1) {
        metallic = texture(uCombinedSamplers[nonuniformEXT(meshInfo.metallicMap)], scaledUv).r;
    }

    // Model blend weights
    // The Disney BSDF is a blend of three models: Metallic BRDF, Dielectric BRDF and Specular BSDF
    float specTrans = clamp(meshInfo.specTrans, 0.0, 1.0);
    float dielectricWeight = (1.0 - metallic) * (1.0 - specTrans);
    float specTransWeight = (1.0 - metallic) * specTrans;

    // Calculate dot products
    float NdotL = max(dot(normNormal, normLightDir), 0.0);
    float NdotV = max(dot(normNormal, normViewDir), 0.0);
    float LdotH = max(dot(normLightDir, normHalfDir), 0.0);
    float NdotH = max(dot(normNormal, normHalfDir), 0.0);

    // IOR-driven specular F0
    float ior = clamp(meshInfo.ior, 1.0, 3.0);
    float etaRelative = isBackFace ? (ior / kAmbientIor) : (kAmbientIor / ior);

    // Incidence reflectance used by the Schlick branch of the Fresnel term
    // The specular tint pulls the dielectric F0 towards the base color chroma, and the metallic branch replaces it
    // with the base color itself
    vec3 albedoTintColor = computeAlbedoTint(albedo);
    vec3 dielectricF0 = computeDielectricF0(ior) * mix(vec3(1.0), albedoTintColor, meshInfo.specularTint);
    vec3 schlickF0 = mix(min(dielectricF0, vec3(1.0)), albedo, metallic);
    vec3 sheenColor = mix(vec3(1.0), albedoTintColor, meshInfo.sheenTint);

    // Extract tangent and bitangent vectors from the TBN matrix.
    vec3 X = normalize(tangentWorldSpace);   // Tangent, anisotropy direction
    vec3 Y = normalize(bitangentWorldSpace); // Bitangent
    float aspect = sqrt(1.0 - meshInfo.anisotropic * 0.9);
    float ax = max(0.001, (roughness * roughness) / aspect);
    float ay = max(0.001, (roughness * roughness) * aspect);

    // Volumetric absorbtion
    // Beer-Lambert with Burley's artist parameterization
    vec3 volumeTransmittance = vec3(1.0);
    if (meshInfo.atDistance > 0.0) {
        vec3 transmittanceColor = clamp(meshInfo.transmittanceColor.rgb, vec3(0.001), vec3(1.0));
        vec3 sigmaA = -log(transmittanceColor) / meshInfo.atDistance;
        volumeTransmittance = exp(-sigmaA * max(meshInfo.thickness, 0.0));
    }

    // Direct Lighting
    // Diffuse: Lambertian plus retro-reflection
    float FL = schlickFresnel(NdotL);
    float FV = schlickFresnel(NdotV);
    float RR = 2.0 * roughness * LdotH * LdotH;

    vec3 fLambert = albedo / PI;
    vec3 diffuseLobe = fLambert * (1.0 - 0.5 * FL) * (1.0 - 0.5 * FV);
    vec3 retroLobe = fLambert * RR * (FL + FV + FL * FV * (RR - 1.0));

    // Sheen calculation
    vec3 sheenLobe = schlickFresnel(LdotH) * meshInfo.sheen * sheenColor; // schlickFresnel(NdotV)

    // Specular reflection
    vec3 FsMetal = schlickF0 + (vec3(1.0) - schlickF0) * schlickFresnel(LdotH);
    vec3 Fs = mix(vec3(fresnelDielectric(LdotH, etaRelative)), FsMetal, metallic);
    float Ds = calculateGTR2Aniso(NdotH, dot(normHalfDir, X),  dot(normHalfDir, Y),  ax, ay);
    float Gs = calculateSmithGGXAniso(NdotL, dot(normLightDir, X), dot(normLightDir, Y), ax, ay) *
               calculateSmithGGXAniso(NdotV, dot(normViewDir, X), dot(normViewDir, Y), ax, ay);
    vec3 specularReflectionLobe = hasValidHalfDir ? Fs * Ds * Gs : vec3(0.0);

    // Clear coat calculation
    float ccNdotH = max(dot(geomNormalWorldSpace, normHalfDir), 0.0);
    float ccNdotL = max(dot(geomNormalWorldSpace, normLightDir), 0.0);
    float ccNdotV = max(dot(geomNormalWorldSpace, normViewDir), 0.0);

    float Fc = mix(0.04, 1.0, schlickFresnel(LdotH));
    float Dc = calculateGTR1(ccNdotH, mix(0.1, 0.001, meshInfo.clearcoatGloss));
    float Gc = calculateSmithGGX(ccNdotL, 0.25) * calculateSmithGGX(ccNdotV, 0.25);
    vec3 clearcoatLobe = vec3(0.25 * meshInfo.clearcoat * Fc * Dc * Gc);

    // Assemble the direct response, the diffuse lobes belong to the dielectric BRDF only
    vec3 disneyBsdf = (diffuseLobe + retroLobe) * dielectricWeight + sheenLobe * (1.0 - metallic) +
                      specularReflectionLobe + clearcoatLobe;

    vec3 directLighting = disneyBsdf * radiance * NdotL;

    // Image-based Lighting (IBL)

    // View-angle Fresnel, same hybrid split as the direct term
    float dielectricFresnelView = fresnelDielectric(NdotV, etaRelative);
    vec3 metalFresnelView = fresnelSchlickRoughness(NdotV, schlickF0, roughness);
    vec3 ambientKS = mix(vec3(dielectricFresnelView), metalFresnelView, metallic);

    // Diffuse irradiance, only for the dielectric BRDF share
    vec3 irradiance = texture(irradianceMap, normNormal).rgb;
    vec3 diffuseIBL = irradiance * albedo * (1.0 - ambientKS) * dielectricWeight;
    vec3 sheenIBL = meshInfo.sheen * sheenColor * irradiance * schlickFresnel(NdotV) * (1.0 - metallic);

    // Anisotropic approximation of the reflection vector and roughness for IBL
    vec3 anisoDir = meshInfo.anisotropic >= 0.0 ? Y : X;
    vec3 crossRaw = cross(anisoDir, normViewDir);
    float crossLen = length(crossRaw);
    vec3 anisoBent = crossLen > 0.001 ? crossRaw / crossLen : normalize(cross(anisoDir, normNormal));
    vec3 bentNormal = normalize(cross(anisoBent, anisoDir));
    vec3 anisoReflect = reflect(-normViewDir, mix(normNormal, bentNormal, abs(meshInfo.anisotropic)));
    float anisoRoughness = mix(roughness, sqrt(ax * ay), abs(meshInfo.anisotropic));

    // Specular reflection IBL
    const float prefilterMaxLod = float(pc.prefilterRoughnessMaxMipCount) - 1.0;
    vec3 prefilteredColor = textureLod(prefilterMap, anisoReflect, anisoRoughness * prefilterMaxLod).rgb;
    vec2 brdfLutSample = texture(brdfLut, vec2(NdotV, anisoRoughness)).rg;
    vec3 specularIBL = prefilteredColor * (ambientKS * brdfLutSample.x + brdfLutSample.y);

    // Clearcoat IBL
    vec3 ccReflDir = reflect(-normViewDir, geomNormalWorldSpace);
    float ccRoughnessIBL = clamp(1.0 - meshInfo.clearcoatGloss, 0.04, 1.0);
    vec3 ccPrefilteredColor = textureLod(prefilterMap, ccReflDir, ccRoughnessIBL * prefilterMaxLod).rgb;
    vec2 ccBrdf = texture(brdfLut, vec2(ccNdotV, ccRoughnessIBL)).rg;
    float ccFresnel = 0.04 + 0.96 * schlickFresnel(ccNdotV); // F0 = 0.04
    vec3 clearcoatIBL = meshInfo.clearcoat * ccPrefilteredColor * (ccFresnel * ccBrdf.x + ccBrdf.y);

    vec3 ambient = diffuseIBL + sheenIBL + specularIBL + clearcoatIBL;

    // Specular transmission from the already rendered scene
    vec3 transmissionLobe = vec3(0.0);

    if (specTransWeight > 0.0) {
        vec3 refractDir = refract(-normViewDir, normNormal, etaRelative);

        // A vector zero means total internal reflection
        if (dot(refractDir, refractDir) > 1e-6)
        {
            // Estimated exit point of the refracted ray
            // Thickness is the only information the technqiue needs about the shape of the object
            vec3 exitPosition = fragPos + normalize(refractDir) * max(meshInfo.thickness, 0.0);

            // Surface absorption by the base color
            vec3 transmittedRadiance = sampleTransmittedRadiance(exitPosition, roughness) * albedo * volumeTransmittance;

            transmissionLobe = transmittedRadiance * specTransWeight * (1.0 - dielectricFresnelView);
        }
    }

    // Composite

    vec3 Lo = directLighting + transmissionLobe;
    if (pc.isIblEnabled == 1) {
        Lo += ambient;
    }

    // The scene color pass writes linear HDR so that the transmission lookup above stays in the same space as the lobes
    if (pc.isToneMapEnabled == 1) {
        Lo = Lo / (Lo + vec3(1.0));
    }

    // The background is resolved inside the shader, so no blending is needed
    outColor = vec4(Lo, 1.0);
}
