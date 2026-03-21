#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUv;

// Constants
#define TILE_SIZE 16
#define MAX_LIGHTS_PER_TILE 64

layout(set = 0, binding = 0) uniform sampler2D gPosition;
layout(set = 0, binding = 1) uniform sampler2D gAlbedo;
layout(set = 0, binding = 2) uniform sampler2D gNormal;

struct PointLightData
{
    vec4 lightPositionIntensity;    // xyz = View-space Position, w = LightIntensity
    vec4 lightColorRadius;          // rgb = Light Color, a = Radius
};

layout(std430, set = 0, binding = 3) readonly buffer PointLightBuffer
{
    PointLightData lights[];
};

struct TileLightList
{
    uint count;
    uint indices[MAX_LIGHTS_PER_TILE];
};

layout(std430, set = 0, binding = 4) readonly buffer TileLightListBuffer {
    TileLightList tileLights[];
};

layout(push_constant) uniform LightPassPushConstants {
    uint tilesX;
} pc;

vec3 calculateLight(PointLightData light, vec3 albedo, vec3 normalView, vec3 fragPosView, vec3 viewDir)
{
    // Constants
    const float specularStrength = 1.0;
    const float shininess = 128.0;

    float radius = light.lightColorRadius.a;
    float intensity = light.lightPositionIntensity.w;

    vec3 lightVec = light.lightPositionIntensity.xyz - fragPosView;
    float dist = length(lightVec);

    if (dist >= radius) {
        return vec3(0.0);
    }

    // Normalize light direction
    vec3 lightDirView = lightVec / dist;

    // Smooth attenuation calculation
    float attenuation = intensity / (dist * dist + 1.0);
    float smoothFactor = 1.0 - (dist / radius);
    smoothFactor = clamp(smoothFactor, 0.0, 1.0);
    smoothFactor *= smoothFactor;
    attenuation *= smoothFactor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalView, lightDirView), 0.0);
    vec3 diffuse = diff * light.lightColorRadius.rgb * albedo;

    // Specular calculation
    vec3 halfDir = normalize(lightDirView + viewDir);
    float NdotH = max(dot(normalView, halfDir), 0.0);
    float specNorm = (shininess + 2.0) / 16.0; // Normalization factor for Blinn-Phong
    float spec = specNorm * pow(NdotH, shininess);
    vec3 specular = spec * light.lightColorRadius.rgb;

    // Final color (attenuation has less effect on specular)
    vec3 finalColor = (diffuse + specular) * attenuation;
    return finalColor;
}

void main()
{
    // Constants
    const float ambientStrength = 0.02;

    // Sampling G-Buffer
    vec3 fragPosView = texture(gPosition, fragUv).xyz;
    vec4 albedoSample = texture(gAlbedo, fragUv);
    vec3 albedo = albedoSample.rgb;
    float opacity = albedoSample.a;

    if (opacity <= 0.001) {
        outColor = vec4(albedo, 1.0);
        return;
    }

    vec3 normalView = texture(gNormal, fragUv).rgb;
    normalView = normalize(normalView);

    // Tile lookup
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    ivec2 tile = pixel / TILE_SIZE;
    uint tileIndex = tile.y * pc.tilesX + tile.x;
    TileLightList tileList = tileLights[tileIndex];

    // Lighting vectors in view space
    vec3 viewDir = normalize(-fragPosView);

    // Ambient calculation
    vec3 ambient = ambientStrength * albedo;

    vec3 resultColor = vec3(0.0);
    for (uint i = 0; i < tileList.count; i++) {
        uint lightIndex = tileList.indices[i];
        resultColor += calculateLight(lights[lightIndex], albedo, normalView, fragPosView, viewDir);
    }
    resultColor += ambient;

    outColor = vec4(resultColor, opacity);
}