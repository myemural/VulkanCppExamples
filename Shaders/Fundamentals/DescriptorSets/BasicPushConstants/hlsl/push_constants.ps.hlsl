// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PushConstants {
    float3 color;
};
[[vk::push_constant]] PushConstants pc;

float4 main() : SV_Target
{
    return float4(pc.color, 1.0);
}