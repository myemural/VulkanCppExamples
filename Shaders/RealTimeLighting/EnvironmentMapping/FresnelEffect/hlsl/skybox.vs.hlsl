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

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
};
[[vk::push_constant]] MeshPushConstants pc;

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float3 directionVector : POSITION;
};

VSOutput main(VSInput input)
{
    // Eliminate translation from view matrix
    float3x3 viewNoTranslation = (float3x3)(pc.view);

    VSOutput output = (VSOutput)0;

    // Vertex position to view direction
    float3 worldPos = mul(transpose(viewNoTranslation), input.pos);
    output.directionVector = worldPos;

    // Z = W
    output.Position = mul(pc.proj, float4(input.pos, 1.0)).xyww;
    return output;
}