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
    [[vk::location(1)]] float3 color : COLOR0;
};

struct VSOutput
{
    [[vk::location(0)]] float3 Color : COLOR0;
    float4 Position : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Color = input.color;
    output.Position = float4(input.pos, 0.0, 1.0);
    return output;
}