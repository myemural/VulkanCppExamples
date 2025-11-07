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

layout (push_constant) uniform PushConstants {
    float time;
} pc;

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size  = imageSize(destImage);
    vec2 uv = vec2(pixel) / vec2(size);

    // Update RGB values over time
    float r = 0.5 + 0.5 * sin(pc.time + uv.x * 6.2831);
    float g = 0.5 + 0.5 * cos(pc.time + uv.y * 6.2831);
    float b = 0.5 + 0.5 * sin(pc.time * 0.5 + (uv.x + uv.y) * 3.1415);

    // Store final pixel
    imageStore(destImage, pixel, vec4(r, g, b, 1.0));
}