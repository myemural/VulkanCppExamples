#version 450
#extension GL_EXT_nonuniform_qualifier : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define LUT_SIZE 64.0
#define PI 3.14159265
#define RECT_AREA_LIGHT_COUNT 4U
#define SPHERE_AREA_LIGHT_COUNT 2U
#define DIRECTIONAL_LIGHT_COUNT 1U

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in mat3 fragTBN;

struct MeshMaterialData
{
    vec4 albedoColor;
    float roughness;
    float metallic;
    float uvScale;
    int albedoMap;
    int roughnessMap;
    int metallicMap;
    int normalMap;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

struct RectangularAreaLightData
{
    vec4 lightColorAndIntensity; // xyz = Light Color, w = Light Intensity
    vec4 lightRectCornerPos[4];  // xyz = Light Rectangle Corner Position
    int isDoubleSided;           // 0: Single-sided, 1: Double-sided
    float _pad[3];
};

struct SphereAreaLightData
{
    vec4 lightPositionAndRadius; // xyz = Light Position, w = Light Radius
    vec4 lightColorAndIntensity; // xyz = Light Color, w = Light Intensity
};

struct DirectionalLightData
{
    vec4 lightDirection;         // xyz = Light Direction
    vec4 lightColorAndIntensity; // xyz = Light Color, w = Light Intensity
};

layout(std430, set = 0, binding = 2) readonly buffer LightBuffer
{
    RectangularAreaLightData rectangularAreaLights[RECT_AREA_LIGHT_COUNT];
    SphereAreaLightData sphereAreaLights[SPHERE_AREA_LIGHT_COUNT];
    DirectionalLightData directionalLights[DIRECTIONAL_LIGHT_COUNT];
};

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];
layout(set = 0, binding = 4) uniform sampler2D uLtc1Lut;
layout(set = 0, binding = 5) uniform sampler2D uLtc2Lut;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
const float LUT_BIAS  = 0.5 / LUT_SIZE;

vec3 calculateNormalWorldSpace(int normalMapIndex, vec2 uv)
{
    vec3 normalWorldSpace;
    if (normalMapIndex != -1) {
        // Normal map sample (tangent space)
        vec3 normalTangent = texture(uCombinedSamplers[nonuniformEXT(normalMapIndex)], uv).rgb;

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
    
    return normalWorldSpace;
}

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

vec2 ltcRemap(vec2 uv)
{
    return uv * LUT_SCALE + LUT_BIAS;
}

// Integrate edge vector with fast acos() calculation
// Reference: https://learnopengl.com/Guest-Articles/2022/Area-Lights
vec3 integrateEdgeVec(vec3 v1, vec3 v2)
{
    // Using built-in acos() function will result flaws
    // Using fitting result for calculating acos()
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
    float b = 3.4175940 + (4.1616724 + y) * y;
    float v = a / b;

    float thetaSintheta = (x > 0.0) ? v : 0.5 * inversesqrt(max(1.0 - x * x, 1e-7)) - v;

    return cross(v1, v2) * thetaSintheta;
}

mat3 buildOrthonormalBasis(vec3 N, vec3 V)
{
    // View-aligned tangent frame (LTC basis construction)
    vec3 T1 = normalize(V - N * dot(V, N));

    // Right-handed orthonormal completion
    vec3 T2 = cross(N, T1);

    return mat3(T1, T2, N);
}

bool isFragmentBehindAreaLight(vec3 fragWorldPos, vec4 points[4])
{
    // Back-face test for rectangular area light
    vec3 dir = points[0].xyz - fragWorldPos;

    // Light plane normal (triangle approximation)
    vec3 lightNormal = cross(points[1].xyz - points[0].xyz, points[3].xyz - points[0].xyz);

    return dot(dir, lightNormal) < 0.0;
}

vec3 integratePolygon(vec3 ltcSpaceVertices[4])
{
    //Spherical polygon integration (solid angle)
    vec3 vectorSum = vec3(0.0);
    vectorSum += integrateEdgeVec(ltcSpaceVertices[0], ltcSpaceVertices[1]);
    vectorSum += integrateEdgeVec(ltcSpaceVertices[1], ltcSpaceVertices[2]);
    vectorSum += integrateEdgeVec(ltcSpaceVertices[2], ltcSpaceVertices[3]);
    vectorSum += integrateEdgeVec(ltcSpaceVertices[3], ltcSpaceVertices[0]);

    return vectorSum;
}

float sampleHorizonClipScale(vec3 vectorSum, bool isBehind)
{
    // LTC horizon-clipping LUT (polygon solid angle correction)
    float len = length(vectorSum);
    float z = vectorSum.z / len;
    z = isBehind ? -z : z; // Flip hemisphere for back-facing evaluation

    vec2 uv = vec2(z * 0.5 + 0.5, len); // Convert to range [0, 1]
    uv = ltcRemap(uv);

    return texture(uLtc2Lut, uv).w;
}

// Evaluate LTC for BRDF approximation
// Reference: https://learnopengl.com/Guest-Articles/2022/Area-Lights
vec3 evaluateLTC(vec3 N, vec3 V, vec3 fragWorldPos, mat3 Minv, vec4 points[4], bool twoSided)
{
    // Construct orthonormal basis around N
    mat3 basis = buildOrthonormalBasis(N, V);

    // Apply LTC transform (precomputed BRDF matrix)
    mat3 ltcTransform = Minv * transpose(basis);

    // Transform area light vertices into LTC space and normalize them
    vec3 ltcSpaceVertices[4];
    for (int i = 0; i < 4; i++)
    {
        vec3 localPos = points[i].xyz - fragWorldPos;
        ltcSpaceVertices[i] = normalize(ltcTransform * localPos);
    }

    // Solid angle integration
    vec3 vectorSum = integratePolygon(ltcSpaceVertices);

    // Check if the shading point is behind the light
    bool isBehind = isFragmentBehindAreaLight(fragWorldPos, points);
    if (isBehind && !twoSided) {
        return vec3(0.0);
    }

    // Horizon-clipped solid angle
    float scale = sampleHorizonClipScale(vectorSum, isBehind);
    return vec3(length(vectorSum) * scale);
}

vec3 getSphereRepresentativeDir(vec3 fragPos, vec3 viewReflect, vec3 sphereCenter, float sphereRadius)
{
    vec3 L = sphereCenter - fragPos;
    float distToCenter = length(L);

    vec3 centerToRay  = dot(L, viewReflect) * viewReflect - L;
    float clampFactor = clamp(sphereRadius / length(centerToRay), 0.0, 1.0);
    vec3 closestPoint = L + centerToRay * clampFactor;

    return normalize(closestPoint);
}

float adjustRoughnessForSphere(float roughness, float sphereRadius, float distToCenter)
{
    return clamp(roughness + sphereRadius / (2.0 * distToCenter), 0.0, 1.0);
}

vec3 calculateTotalRectangularAreaLights(vec3 normalWorldSpace, vec3 viewDirWorldSpace, vec3 albedo, float roughness, float metallic)
{
    // Get LTC values from Look-Up Tables (LUTs)
    float NdotV = max(dot(normalWorldSpace, viewDirWorldSpace), 0.0);
    vec2 ltcUv = vec2(roughness, sqrt(1.0f - NdotV));
    ltcUv = ltcRemap(ltcUv);
    vec4 lut1 = texture(uLtc1Lut, ltcUv);
    vec4 lut2 = texture(uLtc2Lut, ltcUv);

    // Construct inverse M matrix
    mat3 Minv = mat3(
        vec3(lut1.x, 0.0, lut1.y),
        vec3(0.0, 1.0, 0.0),
        vec3(lut1.z, 0.0, lut1.w)
    );

    vec3 finalLo = vec3(0.0);
    for (int i = 0; i < RECT_AREA_LIGHT_COUNT; ++i) {
        // Calculate LTC diffuse and specular
        bool isDoubleSided = rectangularAreaLights[i].isDoubleSided == 0 ? false : true;
        vec3 diffuseLTC = evaluateLTC(normalWorldSpace, viewDirWorldSpace, fragPos, mat3(1), rectangularAreaLights[i].lightRectCornerPos, isDoubleSided);
        vec3 specularLTC = evaluateLTC(normalWorldSpace, viewDirWorldSpace, fragPos, Minv, rectangularAreaLights[i].lightRectCornerPos, isDoubleSided);

        // Calculate Fresnel
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 fresnel = mix(vec3(lut2.y), vec3(lut2.x), F0);
        vec3 kD = (1.0 - fresnel) * (1.0 - metallic) * albedo;

        // Calculate radiance
        vec3 lightColor = rectangularAreaLights[i].lightColorAndIntensity.rgb;
        float lightIntensity = rectangularAreaLights[i].lightColorAndIntensity.a;
        vec3 radiance = lightColor * lightIntensity;

        // Final contribution of the current area light
        vec3 Lo = radiance * (kD * diffuseLTC + fresnel * specularLTC);
        finalLo += Lo;
    }

    return finalLo;
}

vec3 calculateTotalSphereAreaLights(vec3 normalWorldSpace, vec3 viewDirWorldSpace, vec3 albedo, float roughness, float metallic)
{
    vec3 finalLo = vec3(0.0);
    for (int i = 0; i < SPHERE_AREA_LIGHT_COUNT; ++i) {
        // Calculate or get light source related values
        vec3  sphereCenter = sphereAreaLights[i].lightPositionAndRadius.xyz;
        float sphereRadius = sphereAreaLights[i].lightPositionAndRadius.w;
        vec3  lightColor = sphereAreaLights[i].lightColorAndIntensity.xyz;
        float lightIntensity = sphereAreaLights[i].lightColorAndIntensity.w;
        float distToSphereCenter = length(sphereCenter - fragPos);
        float attenuation  = 1.0 / max(distToSphereCenter * distToSphereCenter, 0.01);

        // Calculating light direction and half direction vectors to the sphere area light
        vec3 viewReflect = reflect(-viewDirWorldSpace, normalWorldSpace);
        vec3 normLightDir = getSphereRepresentativeDir(fragPos, viewReflect, sphereCenter, sphereRadius);
        vec3 normHalfDir  = normalize(normLightDir + viewDirWorldSpace);

        // Calculate adjusted roughness and radiance
        float adjustedRoughness = adjustRoughnessForSphere(roughness, sphereRadius, distToSphereCenter);
        vec3 radiance = lightColor * lightIntensity * attenuation;

        // Calculate dot products
        float NdotL = max(dot(normalWorldSpace, normLightDir), 0.0);
        float NdotV = max(dot(normalWorldSpace, viewDirWorldSpace), 0.0);

        // Calculate FDG equations
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 F = fresnelSchlick(max(dot(normHalfDir, viewDirWorldSpace), 0.0), F0);
        float D = distributionGGX(normalWorldSpace, normHalfDir, adjustedRoughness);
        float G = geometrySmith(normalWorldSpace, viewDirWorldSpace, normLightDir, adjustedRoughness);

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
        finalLo += Lo;
    }

    return finalLo;
}

vec3 calculateTotalDirectionalLights(vec3 normalWorldSpace, vec3 viewDirWorldSpace, vec3 albedo, float roughness, float metallic)
{
    vec3 finalLo = vec3(0.0);
    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i) {
        vec3 normLightDir = normalize(-directionalLights[i].lightDirection.xyz);
        vec3 lightColor = directionalLights[i].lightColorAndIntensity.rgb;
        float lightIntensity = directionalLights[i].lightColorAndIntensity.a;
        vec3 normHalfDir = normalize(normLightDir + viewDirWorldSpace);

        // Calculate dot products
        float NdotL = max(dot(normalWorldSpace, normLightDir), 0.0);
        float NdotV = max(dot(normalWorldSpace, viewDirWorldSpace), 0.0);

        // Calculate FDG equations
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 F = fresnelSchlick(max(dot(normHalfDir, viewDirWorldSpace), 0.0), F0);
        float D = distributionGGX(normalWorldSpace, normHalfDir, roughness);
        float G = geometrySmith(normalWorldSpace, viewDirWorldSpace, normLightDir, roughness);

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
        vec3 radiance = lightColor * lightIntensity;
        vec3 Lo = (diffuseBrdf + specular) * radiance * NdotL;
        finalLo += Lo;
    }

    return finalLo;
}

void main()
{
    // Get mesh mat info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    // Calculate scaled UV
    const vec2 scaledUv = fragUv * meshMatInfo.uvScale;
    
    // Normal calculation in world-space (normalized)
    vec3 normalWorldSpace = calculateNormalWorldSpace(meshMatInfo.normalMap, scaledUv);

    // View direction vector calculation in world-space (normalized)
    vec3 viewDirWorldSpace = normalize(pc.cameraPosition.xyz - fragPos);

    // Get correct albedo, roughness and metallic values for fragment
    vec3 albedo = meshMatInfo.albedoColor.rgb;
    if (meshMatInfo.albedoMap != -1) {
        albedo = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.albedoMap)], scaledUv).rgb;
    }

    float roughness = meshMatInfo.roughness;
    if (meshMatInfo.roughnessMap != -1) {
        roughness = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.roughnessMap)], scaledUv).r;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    float metallic = meshMatInfo.metallic;
    if (meshMatInfo.metallicMap != -1) {
        metallic = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.metallicMap)], scaledUv).r;
    }

    // Final color calculation for different types of lights
    vec3 finalLoForRectangularAreaLights = calculateTotalRectangularAreaLights(normalWorldSpace, viewDirWorldSpace, albedo, roughness, metallic);
    vec3 finalLoForSphereAreaLights = calculateTotalSphereAreaLights(normalWorldSpace, viewDirWorldSpace, albedo, roughness, metallic);
    vec3 finalLoForDirectionalLights = calculateTotalDirectionalLights(normalWorldSpace, viewDirWorldSpace, albedo, roughness, metallic);
    vec3 finalColor = finalLoForRectangularAreaLights + finalLoForSphereAreaLights + finalLoForDirectionalLights;

    // Apply simple reinhard tone mapping for HDR to LDR
    vec3 ldrColor = finalColor / (finalColor + vec3(1.0));
    outColor = vec4(ldrColor, 1.0);
}