// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PSInput
{
    [[vk::location(0)]] float3 color : LOCATION0;
};

struct PSOutput
{
    float4 outColor : LOCATION0;
};

[shader("fragment")]
PSOutput main(PSInput input)
{
    PSOutput output;
    output.outColor = float4(input.color, 1.0);
    return output;
}