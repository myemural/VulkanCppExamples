// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

[[vk::binding(0, 0)]] SamplerState accumSampler;
[[vk::binding(0, 0)]] Texture2D accumTex;
[[vk::binding(1, 0)]] SamplerState revealSampler;
[[vk::binding(1, 0)]] Texture2D revealTex;

float4 main(PSInput input) : SV_Target
{
    // Get values from samplers
    float4 accum = accumTex.Sample(accumSampler, input.uv);
    float reveal = revealTex.Sample(revealSampler, input.uv).r;

    // Calculate color and alpha
    float3 backgroundColor = float3(0.0, 0.0, 0.0); /// TODO: Can be taken from CPU or opaque pass.
    float3 color = accum.rgb / max(accum.a, 0.0001);
    float alpha = 1.0 - reveal;
    float3 finalColor = color * alpha + backgroundColor * (1.0 - alpha);
    return float4(finalColor, 1.0);
}
