#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D gPosition;
layout(set = 0, binding = 1) uniform sampler2D gAlbedo;
layout(set = 0, binding = 2) uniform sampler2D gNormal;
layout(set = 0, binding = 3) uniform sampler2D gRoughnessMetallic;

struct PointLightData
{
    vec4 lightPosition;       // xyz = Light Position (View-Space)
    vec4 lightColorIntensity; // xyz = Light Color, w = Color Intensity
};

layout(std430, set = 0, binding = 4) readonly buffer PointLightBuffer
{
    PointLightData lights[];
};

layout(push_constant) uniform LightingPushConstants {
    uint lightCount;
} pc;

const float PI = 3.14159265;

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

vec3 calculateLight(PointLightData light, vec3 albedo, vec3 normalView, vec3 fragPosView, vec3 viewDir, float roughness, float metallic)
{
    vec3 radiance = light.lightColorIntensity.rgb * light.lightColorIntensity.a;
    vec3 lightDirView = normalize(light.lightPosition.xyz - fragPosView);
    float dist = length(light.lightPosition.xyz - fragPosView);
    float attenuation = 1.0 / max(dist * dist, 0.001);
    radiance *= attenuation;

    // Calculate dot products
    float NdotL = max(dot(normalView, lightDirView), 0.0);
    float NdotV = max(dot(normalView, viewDir), 0.0);

    vec3 normHalfDir = normalize(lightDirView + viewDir);

    // Calculate FDG equations
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(normHalfDir, viewDir), 0.0), F0);
    float D = distributionGGX(normalView, normHalfDir, roughness);
    float G = geometrySmith(normalView, viewDir, lightDirView, roughness);

    // Specular calculation
    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL;
    denominator = max(denominator, 0.0001);
    vec3 specular = numerator / denominator;

    // Energy conservation
    vec3 kD = 1.0 - F;
    kD *= (1.0 - metallic);

    // Diffuse calculation
    vec3 diffuseBrdf = kD * albedo / PI;

    // Outgoing light
    vec3 Lo = (diffuseBrdf + specular) * radiance * NdotL;

    return Lo;
}

void main()
{
    // Sampling G-Buffer
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gPosition, 0));

    vec3 fragPosView = texture(gPosition, uv).xyz;
    vec4 albedoSample = texture(gAlbedo, uv);
    vec3 albedo = albedoSample.rgb;
    float opacity = albedoSample.a;

    if (opacity <= 0.001) {
        outColor = vec4(albedo, 1.0);
        return;
    }

    vec3 normalView = texture(gNormal, uv).rgb;
    normalView = normalize(normalView);

    float roughness = texture(gRoughnessMetallic, uv).r;
    float metallic = texture(gRoughnessMetallic, uv).g;

    // Lighting vectors in view space
    vec3 viewDir = normalize(-fragPosView);

    vec3 resultColor = vec3(0.0);
    for (uint i = 0; i < pc.lightCount; i++) {
        resultColor += calculateLight(lights[i], albedo, normalView, fragPosView, viewDir, roughness, metallic);
    }

    // Write HDR output directly
    outColor = vec4(resultColor, opacity);
}
