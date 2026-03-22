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
    uint sampleIndex : SV_SampleIndex;
};

[[vk::binding(0, 0)]] Sampler2D uSampler;

[shader("fragment")]
float4 main(PSInput input) : SV_Target
{
    if (input.sampleIndex == 0) {
        return float4(1.0, 0.0, 0.0, 1.0);
    }

    return uSampler.Sample(input.uv);
}