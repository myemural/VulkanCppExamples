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
layout(set = 0, binding = 4) uniform sampler2D gGeometricNormal;

struct PointLightData
{
    vec4 lightPosition;       // xyz = Light Position (View-Space)
    vec4 lightColorIntensity; // xyz = Light Color, w = Color Intensity
};

layout(std430, set = 0, binding = 5) readonly buffer PointLightBuffer
{
    PointLightData lights[];
};

layout(push_constant) uniform LightingPushConstants {
    uint lightCount;
    uint isCelShadingEnabled; // 0: PBR shading, 1: Cel shading
    uint bandCount;           // Number of the brightness bands of the toon ramp
} pc;

const float PI = 3.14159265;
const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);

// Cel shading parameters
const float kBandSoftness = 0.08; // Width of the soft transition between two bands (in band units)
const float kAmbientIntensity = 0.03; // Constant ambient light (the darkest band is not completely black)
const float kSpecularIntensity = 0.25; // Intensity of the hard specular highlight
const float kShininess = 64.0; // Blinn-phong exponent of the highlight
const float kRampPivot = 0.1; // Brightness at the middle of the toon ramp
const float kRimIntensity = 0.35; // Intensity of the rim light (relative to the lit brightness)

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

vec3 calculatePbrLight(PointLightData light, vec3 albedo, vec3 normalView, vec3 fragPosView, vec3 viewDir, float roughness, float metallic)
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

float luminance(vec3 color)
{
    return dot(color, kLumaRec709);
}

// Toon ramp: Brightness is mapped to [0, 1), quantized into bands with soft transitions and mapped back to HDR
float toonRamp(float brightness)
{
    float bands = float(max(pc.bandCount, 2U));
    float t = brightness / (brightness + kRampPivot);

    float band = floor(t * bands);
    float transition = smoothstep(1.0 - kBandSoftness, 1.0, fract(t * bands));
    float rampedT = min((band + transition) / bands, 0.95);

    return kRampPivot * rampedT / (1.0 - rampedT);
}

vec3 calculateCelShading(vec3 albedo, vec3 normalView, vec3 fragPosView, vec3 viewDir, float roughness)
{
    vec3 diffuseLight = vec3(0.0);
    vec3 specularLight = vec3(0.0);

    // Highlight has a constant shape, smooth surfaces have stronger highlights
    float specularStrength = kSpecularIntensity * (1.0 - roughness);

    for (uint i = 0; i < pc.lightCount; i++) {
        vec3 toLight = lights[i].lightPosition.xyz - fragPosView;
        float dist = length(toLight);
        vec3 lightDir = toLight / max(dist, 1e-4);
        vec3 radiance = lights[i].lightColorIntensity.rgb * lights[i].lightColorIntensity.a / max (dist * dist, 0.001);

        float NdotL = max(dot(normalView, lightDir), 0.0);
        diffuseLight += radiance * NdotL;

        // Hard-edges Blinn-Phong highlight (only on the lit side)
        float NdotH = max(dot(normalView, normalize(lightDir + viewDir)), 0.0);
        float highlight = smoothstep(0.45, 0.55, pow(NdotH, kShininess)) * step(1e-4, NdotL);
        specularLight += radiance * highlight;
    }

    // Brightness of the diffuse light (Lambert BRDF of a white surface) is quantized, the color of the light is kept
    float brightness = luminance(diffuseLight) / PI;
    float rampedBrightness = toonRamp(brightness);
    vec3 rampedDiffuse = brightness > 1e-6 ? diffuseLight / PI * (rampedBrightness / brightness) : vec3(0.0);

    // Rim light on the silhouettes of the lit surfaces
    float NdotV = max(dot(normalView, viewDir), 0.0);
    float rim = smoothstep(0.7, 0.75, 1.0 - NdotV) * kRimIntensity * rampedBrightness;

    return albedo * (rampedDiffuse + kAmbientIntensity + rim) + specularLight * specularStrength;
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

    // Cel shading uses the smooth vertex normals, the details of the normal maps would break the bands
    if (pc.isCelShadingEnabled != 0U) {
        vec3 geometricNormal = normalize(texture(gGeometricNormal, uv).xyz);
        outColor = vec4(calculateCelShading(albedo, geometricNormal, fragPosView, viewDir, roughness), opacity);
        return;
    }

    vec3 resultColor = vec3(0.0);
    for (uint i = 0; i < pc.lightCount; i++) {
        resultColor += calculatePbrLight(lights[i], albedo, normalView, fragPosView, viewDir, roughness, metallic);
    }

    // Write HDR output directly
    outColor = vec4(resultColor, opacity);
}
