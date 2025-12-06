// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PSInput
{
    [[vk::location(0)]] float2 uv : TEXCOORD0;
};

[[vk::binding(0, 0)]] SamplerState uSampler;
[[vk::binding(0, 0)]] Texture2D uImage;

struct UBO
{
    float time;
    float speed;
};
[[vk::binding(1, 0)]] ConstantBuffer<UBO> ubo;

float4 main(PSInput input) : SV_Target
{
    float2 uv = input.uv;
    uv.x += ubo.time * ubo.speed;

    uv.x = frac(uv.x);

    return uImage.Sample(uSampler, uv * 4);
}