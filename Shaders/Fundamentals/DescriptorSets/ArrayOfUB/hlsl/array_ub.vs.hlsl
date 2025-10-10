// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct VSInput
{
    [[vk::location(0)]] float2 pos : POSITION;
};

struct UBO
{
    float4x4 model;
};

[[vk::binding(0, 0)]] ConstantBuffer<UBO> ubo[4];

struct PushConstants {
    nointerpolation int modelIndex;
};
[[vk::push_constant]] PushConstants pc;

struct VSOutput
{
    float4 Position : SV_POSITION;
    nointerpolation int ModelIndex : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Position = mul(ubo[pc.modelIndex].model, float4(input.pos, 0.0, 1.0));
    output.ModelIndex = pc.modelIndex;
    return output;
}