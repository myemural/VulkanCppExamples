#version 450
#extension GL_EXT_nonuniform_qualifier : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define PI 3.14159265

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in mat3 fragTBN;

struct MeshMaterialData
{
    vec4 albedoColor;
    float roughness;
    float metallic;
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
    vec4 lightPositionAndRadius; // xyz = Light Position, w = Light Radius
    vec4 lightColorAndIntensity; // xyz = Light Color, w = Light Intensity
} light;

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N,H),0.0);
    float NdotH2 =NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / max(denom, 0.0000001);
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float denom = NdotV * (1.0 - k) + k;

    return NdotV / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N,V),0.0);
    float NdotL = max(dot(N,L),0.0);
    float ggxV = geometrySchlickGGX(NdotV, roughness);
    float ggxL = geometrySchlickGGX(NdotL, roughness);

    return ggxV * ggxL;
}

// For Specular, find the spherical surface point closest to the reflected ray.
// Reference: https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
vec3 getSphereRepresentativeDir(vec3 fragPos, vec3 viewReflect, vec3 sphereCenter, float sphereRadius)
{
    vec3 L = sphereCenter - fragPos;
    float distToCenter = length(L);

    vec3 centerToRay  = dot(L, viewReflect) * viewReflect - L;
    float clampFactor = clamp(sphereRadius / length(centerToRay), 0.0, 1.0);
    vec3 closestPoint = L + centerToRay * clampFactor;

    return normalize(closestPoint);
}

// Roughness correction to conserve specular's physical energy.
// It simulates the spreading of highlight as the sphere gets larger.
float adjustRoughnessForSphere(float roughness, float sphereRadius, float distToCenter)
{
    return clamp(roughness + sphereRadius / (2.0 * distToCenter), 0.0, 1.0);
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

    // Normalizing normal and view direction
    vec3 normNormal = normalWorldSpace;
    vec3 normViewDir = normalize(pc.cameraPosition.xyz - fragPos);

    // Calculate or get light source related values
    vec3  sphereCenter = light.lightPositionAndRadius.xyz;
    float sphereRadius = light.lightPositionAndRadius.w;
    vec3  lightColor = light.lightColorAndIntensity.xyz;
    float lightIntensity = light.lightColorAndIntensity.w;
    float distToSphereCenter = length(sphereCenter - fragPos);
    float attenuation  = 1.0 / max(distToSphereCenter * distToSphereCenter, 0.01);

    // Calculating light direction and half direction vectors to the sphere area light
    vec3 viewReflect = reflect(-normViewDir, normNormal);
    vec3 normLightDir = getSphereRepresentativeDir(fragPos, viewReflect, sphereCenter, sphereRadius);
    vec3 normHalfDir  = normalize(normLightDir + normViewDir);

    // Calculate adjusted roughness and radiance
    float adjustedRoughness = adjustRoughnessForSphere(roughness, sphereRadius, distToSphereCenter);
    vec3 radiance = lightColor * lightIntensity * attenuation;

    // Calculate dot products
    float NdotL = max(dot(normNormal, normLightDir), 0.0);
    float NdotV = max(dot(normNormal, normViewDir), 0.0);

    // Calculate FDG equations
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(normHalfDir, normViewDir), 0.0), F0);
    float D = distributionGGX(normNormal, normHalfDir, adjustedRoughness);
    float G = geometrySmith(normNormal, normViewDir, normLightDir, adjustedRoughness);

    // Specular calculation
    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL;
    denominator =max(denominator, 0.0001);
    vec3 specular = numerator / denominator;

    // Energy conservation
    vec3 kD = 1.0 - F;
    kD *= (1.0 - metallic);

    // Diffuse calculation
    vec3 diffuseBrdf = kD * albedo / PI;

    // Outgoing light
    vec3 Lo = (diffuseBrdf + specular) * radiance * NdotL;

    // Apply simple reinhard tone mapping for HDR to LDR
    vec3 ldrColor = Lo / (Lo + vec3(1.0));
    outColor = vec4(ldrColor, 1.0);
}