#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;

struct MeshMaterialData
{
    vec4 albedoColor;
    float shininess;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightPosition;  // xyz = Light Position
    vec4 lightColor;     // rgb = Light Color
} light;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

const float PI = 3.14159265;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    // Get mesh info
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];

    // Normalizing fundamental vectors
    vec3 normNormal = normalize(fragNormal);
    vec3 normLightDir = normalize(light.lightPosition.xyz); // Should behave like directional light
    vec3 normViewDir = normalize(pc.cameraPosition.xyz - fragPos);
    vec3 normHalfDir = normalize(normLightDir + normViewDir);

    // Get values from CPU
    vec3 albedo = meshInfo.albedoColor.rgb;
    vec3 radiance = light.lightColor.rgb;

    // Calculate dot products
    float NdotL = max(dot(normNormal, normLightDir), 0.0);

    // Calculate fresnel part
    vec3 F0 = vec3(0.04);
    vec3 F = fresnelSchlick(max(dot(normHalfDir, normViewDir), 0.0), F0);

    // Energy conservation
    vec3 kD = 1.0 - F;

    // Lambert BRDF
    vec3 diffuseBrdf = kD * albedo / PI;

    // Temporary specular (from Blinn-Phong)
    float spec = pow(max(dot(normNormal, normHalfDir), 0.0), meshInfo.shininess);
    vec3 specular = F * spec;

    // Outgoing light
    vec3 Lo = (diffuseBrdf + specular) * radiance * NdotL;

    // Apply simple reinhard tone mapping for HDR to LDR
    vec3 ldrColor = Lo / (Lo + vec3(1.0));
    outColor = vec4(ldrColor, 1.0);
}