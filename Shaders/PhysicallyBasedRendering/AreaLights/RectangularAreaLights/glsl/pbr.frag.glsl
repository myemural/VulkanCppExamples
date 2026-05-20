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
    vec4 lightColorAndIntensity; // xyz = Light Color, w = Light Intensity
    vec4 lightRectCornerPos[4];  // xyz = Light Rectangle Corner Position
    int isDoubleSided;           // 0: Single-sided, 1: Double-sided
} light;

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

    // Normalized view direction vector calculation
    vec3 normViewDir = normalize(pc.cameraPosition.xyz - fragPos);

    // Get correct albedo, roughness and metallic values for fragment
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

    // Get LTC values from Look-Up Tables (LUTs)
    float NdotV = max(dot(normalWorldSpace, normViewDir), 0.0);
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

    // Calculate LTC diffuse and specular
    bool isDoubleSided = light.isDoubleSided == 0 ? false : true;
    vec3 diffuseLTC = evaluateLTC(normalWorldSpace, normViewDir, fragPos, mat3(1), light.lightRectCornerPos, isDoubleSided);
    vec3 specularLTC = evaluateLTC(normalWorldSpace, normViewDir, fragPos, Minv, light.lightRectCornerPos, isDoubleSided);

    // Calculate Fresnel
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 fresnel = mix(vec3(lut2.y), vec3(lut2.x), F0);
    vec3 kD = (1.0 - fresnel) * (1.0 - metallic) * albedo;

    // Calculate radiance
    vec3 lightColor = light.lightColorAndIntensity.rgb;
    float lightIntensity = light.lightColorAndIntensity.a;
    vec3 radiance = lightColor * lightIntensity;

    // Final composition
    vec3 Lo = radiance * (kD * diffuseLTC + fresnel * specularLTC);

    // Apply simple reinhard tone mapping for HDR to LDR
    vec3 ldrColor = Lo / (Lo + vec3(1.0));
    outColor = vec4(ldrColor, 1.0);
}