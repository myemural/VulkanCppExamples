// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct VSOutput
{
    float4 Position : SV_POSITION;
};

VSOutput main(uint vertexID : SV_VertexID)
{
    float2 positions[6] = {
        // Triangle 1
        float2(-1.0, -1.0), // Bottom-left
        float2( 1.0, -1.0), // Bottom-right
        float2(-1.0, 1.0), // Top-left
        // Triangle 2
        float2(-1.0, 1.0), // Top-left
        float2( 1.0, -1.0), // Bottom-right
        float2( 1.0, 1.0)  // Top-right
    };

    VSOutput output;
    output.Position = float4(positions[vertexID], 0.0, 1.0);

    return output;
}
