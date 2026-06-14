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
    [[vk::location(3)]] float4 tangent : TANGENT;
};

struct MeshTransformData
{
    float4x4 model;
    float4x4 normalMatrix;
};
[[vk::binding(0, 0)]] StructuredBuffer<MeshTransformData> meshTransforms;

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    float4x4 reflectionViewProj;
    float4 cameraPosition;
    uint objectId;
};
[[vk::push_constant]] MeshPushConstants pc;

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float3 FragPos : POSITION;
    [[vk::location(1)]] float2 FragUv : TEXCOORD0;
    [[vk::location(2)]] float3x3 FragTBN : NORMAL;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;

    // Get mesh data
    const MeshTransformData meshTransform = meshTransforms[pc.objectId];

    // World-space position and UV
    float4 worldPos = mul(meshTransform.model, float4(input.pos, 1.0f));
    output.FragPos = worldPos.xyz;
    output.FragUv = input.uv;

    // Normal matrix
    float3x3 normalMatrix = (float3x3)meshTransform.normalMatrix;

    // World-space normal and tangent
    float3 N = normalize(mul(normalMatrix, input.normal));
    float3 T = normalize(mul(normalMatrix, input.tangent.xyz));

    // Orthonormalize T
    T = normalize(T - N * dot(N, T));

    // Bitangent calculation (multiplying with bitangent sign value)
    float3 B = cross(N, T) * input.tangent.w;

    output.FragTBN = float3x3(T, B, N);
    output.Position = mul(pc.proj, mul(pc.view, worldPos));
    return output;
}