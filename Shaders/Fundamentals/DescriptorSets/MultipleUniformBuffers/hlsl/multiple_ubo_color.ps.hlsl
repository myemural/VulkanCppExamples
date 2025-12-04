// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct ScreenUBO
{
    float2 screenSize;
};

struct ColorUBO
{
    float3 color;
};

[[vk::binding(0, 0)]] cbuffer screenUbo : register(b0) { ScreenUBO screenUbo; }
[[vk::binding(1, 0)]] cbuffer ubo0 : register(b1) { ColorUBO ubo0; }
[[vk::binding(2, 0)]] cbuffer ubo1 : register(b2) { ColorUBO ubo1; }
[[vk::binding(3, 0)]] cbuffer ubo2 : register(b3) { ColorUBO ubo2; }
[[vk::binding(4, 0)]]cbuffer ubo3 : register(b4) { ColorUBO ubo3; }

float4 main(float4 pos : SV_Position) : SV_Target
{
    float2 uv = pos.xy / screenUbo.screenSize;
    bool left  = uv.x < 0.5;
    bool lower = uv.y < 0.5;

    if ( left &&  lower ) return float4(ubo0.color, 1.0);
    if (!left &&  lower ) return float4(ubo1.color, 1.0);
    if ( left && !lower ) return float4(ubo2.color, 1.0);
    if (!left && !lower ) return float4(ubo3.color, 1.0);

    return float4(0.0, 0.0, 0.0, 1.0);
}