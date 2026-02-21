// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PSInput
{
    [[vk::location(0)]] float3 fragPos : TEXCOORD0;
    [[vk::location(1)]] nointerpolation float3 fragNormal : TEXCOORD1;
};

struct MeshMaterialData
{
    float4 diffuseColor;
};
[[vk::binding(1, 0)]] StructuredBuffer<MeshMaterialData> meshMaterials;

struct LightUBO
{
    float4 lightPosition; // xyz = light position
    float4 lightColor;    // rgb = light color
};

[[vk::binding(2, 0)]] cbuffer Light
{
    LightUBO light;
};

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    uint objectId;
};
[[vk::push_constant]] MeshPushConstants pc;

float4 main(PSInput input) : SV_Target
{
    // Get mesh info
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];

    // Normalizing light direction
    float3 normalizedLightDir = normalize(light.lightPosition.xyz - input.fragPos);

    // Lambert diffuse
    float diff = max(dot(input.fragNormal, normalizedLightDir), 0.0);
    float3 diffuse = diff * light.lightColor.rgb * meshInfo.diffuseColor.rgb;

    return float4(diffuse, 1.0);
}