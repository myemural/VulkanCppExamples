// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct VSInput
{
    [[vk::location(0)]] float3 pos : POSITION;
    uint VertexID : SV_VertexID;
};

struct PushConstants {
    float4x4 mvpMatrix;
};
[[vk::push_constant]] PushConstants pc;

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float2 Uv : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Position = mul(pc.mvpMatrix, float4(input.pos, 1.0));

    uint vertexIndex = input.VertexID % 4;

    // Manual texture coordinates for quad
    switch (vertexIndex) {
        case 0:
            output.Uv = float2(0.0, 0.0);
            break;
        case 1:
            output.Uv = float2(0.0, 1.0);
            break;
        case 2:
            output.Uv = float2(1.0, 0.0);
            break;
        case 3:
            output.Uv = float2(1.0, 1.0);
            break;
    }
    return output;
}