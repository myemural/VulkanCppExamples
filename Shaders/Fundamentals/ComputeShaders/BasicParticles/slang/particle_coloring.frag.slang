// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PSInput
{
    float lifetime : LIFETIME;
};

[shader("fragment")]
float4 main(PSInput input) : SV_Target
{
    float red = clamp(input.lifetime * 2.0, 0.0, 1.0);
    float3 baseColor = float3(1.0 - red, 0.0, 0.0);
    float alpha = clamp(input.lifetime, 0.0, 1.0);

    return float4(baseColor, alpha);
}