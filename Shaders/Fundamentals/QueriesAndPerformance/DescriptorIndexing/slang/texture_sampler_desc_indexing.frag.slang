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

[[vk::binding(1, 0)]] Sampler2D uSamplers[];

[shader("fragment")]
float4 main(PSInput input) : SV_Target
{
    return uSamplers[input.textureIndex].Sample(input.uv);
}