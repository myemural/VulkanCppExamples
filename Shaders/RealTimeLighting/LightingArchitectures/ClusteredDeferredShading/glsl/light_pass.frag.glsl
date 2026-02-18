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
#define Z_SLICE_COUNT 16

layout(set = 0, binding = 0) uniform sampler2D gPosition;
layout(set = 0, binding = 1) uniform sampler2D gAlbedo;
layout(set = 0, binding = 2) uniform sampler2D gNormal;

struct PointLightData
{
    vec4 lightPosition;    // xyz = View-space Position
    vec4 lightColorRadius; // rgb = Light Color, a = Radius
};

layout(std430, set = 0, binding = 3) readonly buffer PointLightBuffer {
    PointLightData lights[];
};

struct ClusterHeader
{
    uint offset;
    uint count;
};

layout(std430, set = 0, binding = 4) readonly buffer ClusterHeaderBuffer
{
    ClusterHeader clusterHeaders[];
};

layout(std430, set=0, binding = 5) readonly buffer GlobalLightIndexBuffer
{
    uint globalLightIndices[];
};

layout(push_constant) uniform LightPassPushConstants {
    uint tilesX;
    float nearPlane;
    float farPlane;
} pc;

vec3 calculateLight(PointLightData light, vec3 albedo, vec3 normalView, vec3 fragPosView, vec3 viewDir)
{
    // Constants
    const float specularStrength = 1.0;
    const float shininess = 64.0;

    float radius = light.lightColorRadius.a;

    vec3 lightVec = light.lightPosition.xyz - fragPosView;
    float dist = length(lightVec);

    // Normalize light direction
    vec3 lightDirView = lightVec / dist;

    // Radius-based attenuation
    float d2 = dist * dist;
    float r2 = radius * radius;

    float attenuation = max(0.0, 1.0 - (d2 / r2));
    attenuation *= attenuation;
    attenuation /= (1.0 + d2);

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalView, lightDirView), 0.0);
    vec3 diffuse = diff * light.lightColorRadius.rgb * albedo;

    // Specular calculation
    vec3 halfDir = normalize(lightDirView + viewDir);
    float spec = pow(max(dot(normalView, halfDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * light.lightColorRadius.rgb;

    // Final color (attenuation has less effect on specular)
    vec3 finalColor = (diffuse * attenuation) + (specular * sqrt(attenuation));
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

    // Cluster lookup
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    ivec2 tile  = pixel / TILE_SIZE;

    float viewZ = -fragPosView.z;   // View-space forward
    float logZ = (log(viewZ) - log(pc.nearPlane)) / (log(pc.farPlane) - log(pc.nearPlane));
    uint zSlice = uint(clamp(logZ * Z_SLICE_COUNT, 0.0, float(Z_SLICE_COUNT - 1)));
    uint clusterIndex = zSlice + Z_SLICE_COUNT * (tile.y * pc.tilesX + tile.x);
    ClusterHeader header = clusterHeaders[clusterIndex];

    vec3 normalView = texture(gNormal, fragUv).rgb;
    normalView = normalize(normalView);

    // Lighting vectors in view space
    vec3 viewDir = normalize(-fragPosView);

    // Ambient calculation
    vec3 ambient = ambientStrength * albedo;

    vec3 resultColor = vec3(0.0);
    for (uint i = 0; i < header.count; i++) {
        uint lightIndex = globalLightIndices[header.offset + i];
        resultColor += calculateLight(lights[lightIndex], albedo, normalView, fragPosView, viewDir);
    }
    resultColor += ambient;

    outColor = vec4(resultColor, opacity);
}