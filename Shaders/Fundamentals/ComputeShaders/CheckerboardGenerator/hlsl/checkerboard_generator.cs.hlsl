// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

// Output image (binding = 0)
[[vk::image_format("rgba8")]] RWTexture2D<unorm float4> destImage : register(u0);

struct PushConstants
{
    int tileSize;
};
[[vk::push_constant]] PushConstants pc;

[numthreads(16, 16, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    uint2 gid = dtid.xy;
    uint width, height;
    destImage.GetDimensions(width, height);
    uint2 size = uint2(width, height);

    // Bounds check
    if (gid.x >= size.x || gid.y >= size.y) {
        return;
    }

    int tx = int(gid.x) / pc.tileSize;
    int ty = int(gid.y) / pc.tileSize;

    bool even = ((tx + ty) & 1) == 0;

    float4 colorA = float4(1.0, 1.0, 1.0, 1.0); // White
    float4 colorB = float4(0.0, 0.0, 0.0, 1.0); // Black
    float4 color  = even ? colorA : colorB;

    destImage[gid] = color;
}