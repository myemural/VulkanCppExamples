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

cbuffer ubo : register(b0, space0) { UBO ubo; }

struct VSOutput
{
    float4 Position : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Position = mul(ubo.model, float4(input.pos, 0.0, 1.0));
    return output;
}