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

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    uint objectId;
};
[[vk::push_constant]] MeshPushConstants pc;

float4 main(PSInput input) : SV_Target
{
    // Hardcoded white color for light objects
    return float4(1.0, 1.0, 1.0, 1.0);
}