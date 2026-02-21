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
    [[vk::location(1)]] float3 normal : NORMAL;
};

struct MeshTransformData
{
    float4x4 model;
    float4x4 normalMatrix;
};
[[vk::binding(0, 0)]] StructuredBuffer<MeshTransformData> meshTransforms;

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

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float3 VertexColor : COLOR;
};

VSOutput main(VSInput input)
{
    // Get mesh info
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];
    const float4x4 model = meshTransforms[pc.objectId].model;
    const float3x3 normalMatrix = (float3x3)meshTransforms[pc.objectId].normalMatrix;

    // World-space position
    float3 worldPos = mul(model, float4(input.pos, 1.0f)).xyz;

    // World-space normal
    float3 worldNormal = normalize(mul(normalMatrix, input.normal));

    // Normalizing light direction
    float3 normalizedLightDir = normalize(light.lightPosition.xyz - worldPos);

    // Lambert diffuse
    float diff = max(dot(worldNormal, normalizedLightDir), 0.0);
    float3 diffuse = diff * light.lightColor.rgb * meshInfo.diffuseColor.rgb;

    VSOutput output = (VSOutput)0;
    output.VertexColor = diffuse;
    output.Position = mul(pc.proj, mul(pc.view, float4(worldPos, 1.0)));
    return output;
}