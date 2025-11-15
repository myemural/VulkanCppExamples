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
    float2 center;
    float loopTime;
    float zoomSpeed;
    float time;
};
[[vk::push_constant]] PushConstants pc;

// Convert HSV to RGB
float3 ConvertHsvToRgb(float3 hsv)
{
    float3 rgb = clamp(abs(fmod(hsv.x * 6.0 + float3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return hsv.z * lerp(float3(1.0, 1.0, 1.0), rgb, hsv.y);
}

[numthreads(16, 16, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    uint2 pix = dtid.xy;
    uint width, height;
    destImage.GetDimensions(width, height);
    uint2 size = uint2(width, height);

    // Bounds check
    if (pix.x >= size.x || pix.y >= size.y) {
        return;
    }

    //  Zoom cycle calculation
    float cycleTime = fmod(pc.time, pc.loopTime);
    float scale = 4.0 / float(size.x) * exp(-pc.zoomSpeed * cycleTime);

    // Coordinates calculation
    float x0 = (float(pix.x) - size.x * 0.5) * scale + pc.center.x;
    float y0 = (float(pix.y) - size.y * 0.5) * scale + pc.center.y;

    float x = 0.0;
    float y = 0.0;
    int iter = 0;
    int maxIter = 900;

    while (x * x + y * y <= 4.0 && iter < maxIter)
    {
        float xtemp = x * x - y * y + x0;
        y = 2.0 * x * y + y0;
        x = xtemp;
        iter++;
    }

    // Color calculation
    float val = (iter == maxIter) ? 0.0 : float(iter) / float(maxIter);
    float hue = 0.65 + 0.4 * val + 0.1 * sin(pc.time * 0.2);
    float sat = 0.9;
    float bri = (iter == maxIter) ? 0.0 : 1.0;
    float3 rgb = ConvertHsvToRgb(float3(frac(hue), sat, bri));

    destImage[pix] = float4(rgb, 1.0);
}