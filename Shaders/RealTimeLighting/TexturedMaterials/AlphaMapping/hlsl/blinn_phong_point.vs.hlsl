// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct VSInput
{
    [[vk::location(0)]] float3 pos : POSITION;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
    [[vk::location(2)]] float3 normal : NORMAL;
};

struct MeshData
{
    float4x4 model;
    float4x4 normalMatrix;
    float4 diffuseColor;
    float4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float opacity;
    float reflectivity;
    int diffuseMap;
    int specularMap;
    int normalMap;
    int emissiveMap;
    int shininessMap;
    int opacityMap;
    int aoMap;
    int heightMap;
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

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float3 FragPos : POSITION;
    [[vk::location(1)]] float2 FragUv : TEXCOORD0;
    [[vk::location(2)]] float3 FragNormal : NORMAL;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;

    float4x4 model = meshes[pc.objectId].model;
    float4 worldPos = mul(model, float4(input.pos, 1.0f));
    output.FragPos = worldPos.xyz;

    output.FragUv = input.uv;

    float3x3 normalMatrix = (float3x3)meshes[pc.objectId].normalMatrix;
    output.FragNormal = mul(normalMatrix, input.normal);

    output.Position = mul(pc.proj, mul(pc.view, worldPos));
    return output;
}