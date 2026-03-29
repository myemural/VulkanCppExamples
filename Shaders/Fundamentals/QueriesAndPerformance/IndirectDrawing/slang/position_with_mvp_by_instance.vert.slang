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
    uint instanceIndex : SV_InstanceID;
};

[[vk::binding(0, 0)]]
StructuredBuffer<float4x4> ObjectSSBO;

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float2 Uv : TEXCOORD0;
};

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Position = mul(ObjectSSBO[input.instanceIndex], float4(input.pos, 1.0));
    output.Uv = input.uv;
    return output;
}