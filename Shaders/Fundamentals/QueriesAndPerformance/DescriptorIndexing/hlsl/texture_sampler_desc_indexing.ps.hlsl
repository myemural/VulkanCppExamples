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
    [[vk::location(1)]] nointerpolation uint textureIndex : TEXCOORD1;
};

[[vk::binding(1, 0)]] SamplerState uSamplers[];
[[vk::binding(1, 0)]] Texture2D uImages[];

float4 main(PSInput input) : SV_Target
{
    return uImages[input.textureIndex].Sample(uSamplers[input.textureIndex], input.uv);
}