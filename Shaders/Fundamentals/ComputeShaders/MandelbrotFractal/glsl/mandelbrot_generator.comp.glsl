#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba8) uniform writeonly image2D destImage;

layout(push_constant) uniform Push {
    vec2 center;
    float loopTime;
    float zoomSpeed;
    float time;
} pc;

// HSV to RGB conversion
vec3 ConvertHsvToRgb(vec3 hsv)
{
    vec3 rgb = clamp(abs(mod(hsv.x*6.0 + vec3(0.0,4.0,2.0), 6.0) - 3.0)-1.0, 0.0, 1.0);
    return hsv.z * mix(vec3(1.0), rgb, hsv.y);
}

void main()
{
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(destImage);
    if (pix.x >= size.x || pix.y >= size.y) {
        return;
    }

    // Zoom cycle calculation
    float cycleTime = mod(pc.time, pc.loopTime);
    float scale = 4.0 / float(size.x) * exp(-pc.zoomSpeed * cycleTime);

    // Coordinates calculation
    float x0 = (pix.x - size.x * 0.5) * scale + pc.center.x;
    float y0 = (pix.y - size.y * 0.5) * scale + pc.center.y;

    float x = 0.0;
    float y = 0.0;
    int iter = 0;
    int maxIter = 900;

    while (x*x + y*y <= 4.0 && iter < maxIter) {
        float xtemp = x*x - y*y + x0;
        y = 2.0*x*y + y0;
        x = xtemp;
        iter++;
    }

    // Color calculation
    float val = iter == maxIter ? 0.0 : float(iter) / float(maxIter);
    float hue = 0.65 + 0.4 * val + 0.1 * sin(pc.time * 0.2);
    float sat = 0.9;
    float bri = (iter == maxIter) ? 0.0 : 1.0;
    vec3 rgb = ConvertHsvToRgb(vec3(fract(hue), sat, bri));

    imageStore(destImage, pix, vec4(rgb, 1.0));
}