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

[[vk::binding(0, 0)]] SamplerState opaqueSampler;
[[vk::binding(0, 0)]] Texture2D opaqueTex;
[[vk::binding(1, 0)]] SamplerState peelLayersSampler[];
[[vk::binding(1, 0)]] Texture2D peelLayersTex[];

struct CompositionPushConstants
{
    int layerCount;
};
[[vk::push_constant]] CompositionPushConstants pc;

float4 main(PSInput input) : SV_Target
{
    float4 accum = opaqueTex.Sample(opaqueSampler, input.uv);
    for(int i = pc.layerCount - 1; i >= 0; --i) {
        uint index = NonUniformResourceIndex(i);
        float4 c = peelLayersTex[index].Sample(peelLayersSampler[index], input.uv);
        accum.rgb = c.rgb * c.a + accum.rgb * (1.0 - c.a);
        accum.a   = c.a + accum.a * (1.0 - c.a);
    }

    return accum;
}
