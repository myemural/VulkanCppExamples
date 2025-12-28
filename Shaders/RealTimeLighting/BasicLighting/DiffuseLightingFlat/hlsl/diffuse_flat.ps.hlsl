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

struct MeshData
{
    float4x4 model;
    float4x4 normalMatrix;
    float4 diffuseColor;
    float4 specularColor;
    float  ambientStrength;
    float  shininess;
    float  specularStrength;
    float  opacity;
};
[[vk::binding(0, 0)]] StructuredBuffer<MeshData> meshes : register(t0);

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    float4 cameraPosition;
    uint objectId;
};
[[vk::push_constant]] MeshPushConstants pc;

struct LightUBO
{
    float4 lightPosition; // xyz = light position
    float4 lightColor;    // rgb = light color
};

[[vk::binding(1, 0)]]
cbuffer Light : register(b1)
{
    LightUBO light;
};

float4 main(PSInput input) : SV_Target
{
    // Get mesh info
    MeshData meshInfo = meshes[pc.objectId];

    // Normalizing light direction
    float3 normalizedLightDir = normalize(light.lightPosition.xyz - input.fragPos);

    // Lambert diffuse
    float diff = max(dot(input.fragNormal, normalizedLightDir), 0.0);
    float3 diffuse = diff * light.lightColor.rgb * meshInfo.diffuseColor.rgb;

    return float4(diffuse, meshInfo.opacity);
}