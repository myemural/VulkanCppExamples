// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define TILE_SIZE 16
#define Z_SLICE_COUNT 16

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

[[vk::binding(0, 0)]] SamplerState gPositionSampler;
[[vk::binding(0, 0)]] Texture2D gPositionTex;
[[vk::binding(1, 0)]] SamplerState gAlbedoSampler;
[[vk::binding(1, 0)]] Texture2D gAlbedoTex;
[[vk::binding(2, 0)]] SamplerState gNormalSampler;
[[vk::binding(2, 0)]] Texture2D gNormalTex;

struct PointLightData
{
    float4 lightPositionIntensity;    // xyz = View-space Position, w = LightIntensity
    float4 lightColorRadius;          // rgb = Light Color, a = Radius
};
[[vk::binding(3, 0)]] StructuredBuffer<PointLightData> lights;

struct ClusterHeader
{
    uint offset;
    uint count;
};
[[vk::binding(4, 0)]] StructuredBuffer<ClusterHeader> clusterHeaders;

[[vk::binding(5, 0)]] StructuredBuffer<uint> globalLightIndices;

struct MeshPushConstants
{
    uint tilesX;
    float nearPlane;
    float farPlane;
};
[[vk::push_constant]] MeshPushConstants pc;

float3 calculateLight(PointLightData light, float3 albedo, float3 normalView, float3 fragPosView, float3 viewDir)
{
    // Constants
    const float specularStrength = 1.0;
    const float shininess = 64.0;

    float radius = light.lightColorRadius.a;
    float intensity = light.lightPositionIntensity.w;

    float3 lightVec = light.lightPositionIntensity.xyz - fragPosView;
    float dist = length(lightVec);

    // Normalize light direction
    float3 lightDirView = lightVec / dist;

    // Smooth attenuation calculation
    float attenuation = intensity / (dist * dist + 1.0);
    float smoothFactor = 1.0 - (dist / radius);
    smoothFactor = clamp(smoothFactor, 0.0, 1.0);
    smoothFactor *= smoothFactor;
    attenuation *= smoothFactor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalView, lightDirView), 0.0);
    float3 diffuse = diff * light.lightColorRadius.rgb * albedo;

    // Specular calculation
    float3 halfDir = normalize(lightDirView + viewDir);
    float NdotH = max(dot(normalView, halfDir), 0.0);
    float specNorm = (shininess + 2.0) / 16.0; // Normalization factor for Blinn-Phong
    float spec = specNorm * pow(NdotH, shininess);
    float3 specular = spec * light.lightColorRadius.rgb;

    // Final color (attenuation has less effect on specular)
    float3 finalColor = (diffuse + specular) * attenuation;
    return finalColor;
}

float4 main(PSInput input) : SV_Target
{
    // Constants
    const float ambientStrength = 0.02;

    // Sampling G-Buffer
    uint width;
    uint height;
    gPositionTex.GetDimensions(width, height);
    float2 uv = input.position.xy / float2(width, height);

    float3 fragPosView = gPositionTex.Sample(gPositionSampler, uv).xyz;
    float4 albedoSample = gAlbedoTex.Sample(gAlbedoSampler, uv);
    float3 albedo = albedoSample.rgb;
    float opacity = albedoSample.a;

    if (opacity <= 0.001) {
        return float4(albedo, 1.0);
    }

    float3 normalView = gNormalTex.Sample(gNormalSampler, uv).rgb;
    normalView = normalize(normalView);

    // Cluster lookup
    int2 pixel = int2(input.position.xy);
    int2 tile = pixel / TILE_SIZE;

    float viewZ = -fragPosView.z;   // View-space forward
    float logZ = (log(viewZ) - log(pc.nearPlane)) / (log(pc.farPlane) - log(pc.nearPlane));
    uint zSlice = uint(clamp(logZ * Z_SLICE_COUNT, 0.0, float(Z_SLICE_COUNT - 1)));
    uint clusterIndex = zSlice + Z_SLICE_COUNT * (tile.y * pc.tilesX + tile.x);
    ClusterHeader header = clusterHeaders[clusterIndex];

    // Lighting vectors in view space
    float3 viewDir = normalize(-fragPosView);

    // Ambient calculation
    float3 ambient = ambientStrength * albedo;

    float3 resultColor = 0.0;
    for (uint i = 0; i < header.count; i++) {
        uint lightIndex = globalLightIndices[header.offset + i];
        resultColor += calculateLight(lights[lightIndex], albedo, normalView, fragPosView, viewDir);
    }
    resultColor += ambient;

    return float4(resultColor, opacity);
}
