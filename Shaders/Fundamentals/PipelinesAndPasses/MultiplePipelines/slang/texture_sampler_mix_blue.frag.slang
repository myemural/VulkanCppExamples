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

[[vk::binding(0, 0)]] Sampler2D uSampler;

[shader("fragment")]
float4 main(PSInput input) : SV_Target
{
    float4 texColor = uSampler.Sample(input.uv);
    float4 targetBlue = float4(0.0, 0.0, 1.0, texColor.a);
    float blend = 0.5;
    return lerp(texColor, targetBlue, blend);
}