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
    float time;
};
[[vk::push_constant]] PushConstants pc;

[numthreads(16, 16, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    uint2 pixel = dtid.xy;
    uint2 size;
    destImage.GetDimensions(size.x, size.y);
    float2 uv = float2(pixel) / float2(size);

    // Update RGB values over time
    float r = 0.5 + 0.5 * sin(pc.time + uv.x * 6.2831);
    float g = 0.5 + 0.5 * cos(pc.time + uv.y * 6.2831);
    float b = 0.5 + 0.5 * sin(pc.time * 0.5 + (uv.x + uv.y) * 3.1415);

    // Store final pixel
    destImage[pixel] = float4(r, g, b, 1.0);
}