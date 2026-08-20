#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

// One reduce pass: Downsamples srcMip (mip N) into dstMip (mip N+1), each dispatched separately
layout(set = 0, binding = 0, r32f) uniform readonly image2D srcMip;
layout(set = 0, binding = 1, r32f) uniform writeonly image2D dstMip;

// Clamp-to-edge sampling so odd-sized mips don't read out of bounds
float loadClamped(ivec2 coord, ivec2 srcSize)
{
    return imageLoad(srcMip, clamp(coord, ivec2(0), srcSize - 1)).r;
}

void main()
{
    const ivec2 dstTexel = ivec2(gl_GlobalInvocationID.xy);
    const ivec2 dstSize = imageSize(dstMip);

    if (any(greaterThanEqual(dstTexel, dstSize))) {
        return;
    }

    const ivec2 srcTexel = dstTexel * 2;
    const ivec2 srcSize = imageSize(srcMip);

    // Standard 2x2 max-reduction (not average)
    float maxDepth = max(max(loadClamped(srcTexel + ivec2(0, 0), srcSize),
                             loadClamped(srcTexel + ivec2(1, 0), srcSize)),
                         max(loadClamped(srcTexel + ivec2(0, 1), srcSize),
                             loadClamped(srcTexel + ivec2(1, 1), srcSize)));

    // When srcSize is odd, a 2x2 sample skips a trailing row/column of texels
    const bool isSrcWidthOdd = (srcSize.x & 1) != 0;
    const bool isSrcHeightOdd = (srcSize.y & 1) != 0;

    if (isSrcWidthOdd) {
        maxDepth = max(maxDepth, max(loadClamped(srcTexel + ivec2(2, 0), srcSize),
                                     loadClamped(srcTexel + ivec2(2, 1), srcSize)));
    }

    if (isSrcHeightOdd) {
        maxDepth = max(maxDepth, max(loadClamped(srcTexel + ivec2(0, 2), srcSize),
                                     loadClamped(srcTexel + ivec2(1, 2), srcSize)));
    }

    if (isSrcWidthOdd && isSrcHeightOdd) {
        // Corner texel missed by both the width and height extensions above
        maxDepth = max(maxDepth, loadClamped(srcTexel + ivec2(2, 2), srcSize));
    }

    imageStore(dstMip, dstTexel, vec4(maxDepth, 0.0, 0.0, 0.0));
}
