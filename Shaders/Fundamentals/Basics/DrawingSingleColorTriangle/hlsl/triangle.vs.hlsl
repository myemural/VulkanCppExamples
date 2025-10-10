// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

static const float2 positions[3] = {
    float2(0.0, -0.5), // Bottom
    float2(0.5, 0.5),  // Right-top
    float2(-0.5, 0.5)  // Left-top
};

struct VSOutput
{
    float4 Position : SV_POSITION;
};

VSOutput main(uint vertexId : SV_VertexID)
{
    VSOutput output = (VSOutput)0;
    float2 pos = positions[vertexId % 3];
    output.Position = float4(pos, 0.0, 1.0);
    return output;
}