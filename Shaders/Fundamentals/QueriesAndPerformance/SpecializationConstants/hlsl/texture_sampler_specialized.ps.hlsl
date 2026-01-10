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

// 0 = Flat color (blue)
// 1 = Texture
// 2 = Mixture of color (blue) and texture
[[vk::constant_id(0)]] const int MODE = 0;

[[vk::binding(1, 0)]] SamplerState uSampler;
[[vk::binding(1, 0)]] Texture2D uImage;

float4 main(PSInput input) : SV_Target
{
    if (MODE == 0)
    {
        return float4(0.0, 0.0, 1.0, 1.0);
    }
    else if (MODE == 1)
    {
        return uImage.Sample(uSampler, input.uv);
    }
    else if (MODE == 2)
    {
        float4 textureColor = uImage.Sample(uSampler, input.uv);
        return lerp(float4(0.0, 0.0, 1.0, 1.0), textureColor, 0.5);
    }

    // Fallback (red)
    return float4(1.0, 0.0, 0.0, 1.0);
}