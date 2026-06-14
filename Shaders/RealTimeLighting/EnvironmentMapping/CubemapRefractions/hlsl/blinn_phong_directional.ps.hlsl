// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PSInput
{
    [[vk::location(0)]] float3 fragPos : POSITION;
    [[vk::location(1)]] float2 fragUv : TEXCOORD0;
    [[vk::location(2)]] float3x3 fragTBN : NORMAL;
};

struct MeshMaterialData
{
    float4 diffuseColor;
    float  ambientStrength;
};
[[vk::binding(1, 0)]] StructuredBuffer<MeshMaterialData> meshMaterials;

struct LightUBO
{
    float4 lightDirection; // xyz = Light Direction
    float4 lightColor;     // rgb = Light Color
};

[[vk::binding(2, 0)]] cbuffer Light
{
    LightUBO light;
};

[[vk::binding(3, 0)]] SamplerState uEnvironmentMapSampler;
[[vk::binding(3, 0)]] TextureCube uEnvironmentMapTexture;

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    float4 cameraPosition;
    uint objectId;
};
[[vk::push_constant]] MeshPushConstants pc;

float4 main(PSInput input) : SV_Target
{
    // Get mesh info
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];

    float3 diffuseColor = meshInfo.diffuseColor.rgb;

    // Normal map calculation
    float3 normalWorldSpace = normalize(input.fragTBN[2]);

    // Normalizing view direction (camera position)
    float3 normalizedView = normalize(pc.cameraPosition.xyz - input.fragPos);

    // Refraction calculation
    const float ior = 1.5; // For glass material
    const float eta = 1.0 / ior; // For air

    // Do not consider exit pass refraction
    float3 refractDir = refract(normalizedView, normalWorldSpace, eta);
    refractDir.z *= -1.0;
    float3 refractionColor = uEnvironmentMapTexture.Sample(uEnvironmentMapSampler, refractDir).rgb;

    // Ambient calculation
    float3 ambient = meshInfo.ambientStrength * diffuseColor;

    // Final color
    const float refractionFactor = 0.8;
    float3 finalColor = lerp(ambient, refractionColor, refractionFactor);

    return float4(finalColor, 1.0);
}