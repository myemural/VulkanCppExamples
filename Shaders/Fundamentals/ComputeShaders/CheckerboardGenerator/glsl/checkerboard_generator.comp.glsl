#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba8) uniform writeonly image2D outImage;

layout(push_constant) uniform PushConstants {
    int tileSize;
} pc;

void main() {
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outImage);

    if (gid.x >= size.x || gid.y >= size.y) {
        return;
    }

    int tx = gid.x / pc.tileSize;
    int ty = gid.y / pc.tileSize;

    bool even = ((tx + ty) & 1) == 0;
    vec4 colorA = vec4(1.0); // White
    vec4 colorB = vec4(0.0, 0.0, 0.0, 1.0); // Black
    vec4 color = even ? colorA : colorB;

    imageStore(outImage, gid, color);
}