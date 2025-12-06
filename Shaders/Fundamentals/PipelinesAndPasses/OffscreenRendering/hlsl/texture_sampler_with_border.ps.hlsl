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
[[vk::binding(0, 0)]] Texture2D uOffscreenTex;

float4 main(PSInput input) : SV_Target
{
    float border = 0.01; // Border width
    if((input.uv.x < border) || (input.uv.x > 1.0 - border) || (input.uv.y < border) || (input.uv.y > 1.0 - border)) {
        return float4(1.0, 0.0, 0.0, 1.0); // Border color
    }

    return uOffscreenTex.Sample(uSampler, input.uv);
}