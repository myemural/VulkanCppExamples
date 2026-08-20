#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

// Source: The actual rasterized scene depth buffer
layout(set = 0, binding = 0) uniform sampler2D sceneDepth;

// Destination: Mip 0 of the Hi-Z pyramid
layout(set = 0, binding = 1, r32f) uniform writeonly image2D hizMipZero;

void main()
{
    const ivec2 texel = ivec2(gl_GlobalInvocationID.xy);
    const ivec2 mipSize = imageSize(hizMipZero);

    // The dispatch is rounded up to whole workgroups, so the edges have to be discarded
    if (any(greaterThanEqual(texel, mipSize))) {
        return;
    }

    // Mip 0 is a one-to-one copy
    const float depth = texelFetch(sceneDepth, texel, 0).r;

    imageStore(hizMipZero, texel, vec4(depth, 0.0, 0.0, 0.0));
}
