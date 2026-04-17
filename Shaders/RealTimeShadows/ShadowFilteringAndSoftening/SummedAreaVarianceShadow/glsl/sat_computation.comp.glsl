#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0) uniform sampler2D inputShadowMap;

layout(binding = 1, rg32f) uniform image2D satImage;

layout(push_constant) uniform PushConstants {
    uint pass; // 0 = Horizontal, 1 = Vertical
    uint shadowMapSize; // Shadow map size
} pc;

void main()
{
    uint lineIdx = gl_GlobalInvocationID.x;
    if (lineIdx >= pc.shadowMapSize) {
        return;
    }

    if (pc.pass == 0) {
        // Horizontal pass
        vec2 acc = vec2(0.0);
        for (uint i = 0; i < pc.shadowMapSize; ++i) {
            ivec2 coord = ivec2(i, lineIdx);
            vec2 val = texelFetch(inputShadowMap, coord , 0).rg;

            acc += val;

            imageStore(satImage, coord, vec4(acc, 0.0, 1.0));
        }
    } else {
        // Vertical pass
        vec2 acc = vec2(0.0);
        for (uint i = 0; i < pc.shadowMapSize; ++i) {
            ivec2 coord = ivec2(lineIdx, i);
            vec2 val = imageLoad(satImage, coord).rg;

            acc += val;

            imageStore(satImage, coord, vec4(acc, 0.0, 1.0));
        }
    }
}
