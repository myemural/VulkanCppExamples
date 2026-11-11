#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outVelocity;

layout(set = 0, binding = 0) uniform sampler2D uSourceImage; // TileMax image (haşf-velocity in pixels)

layout(push_constant) uniform VelocityTilePushConstants {
    float blurScale;        // Not used in this pass
    float maxBlurRadius;    // Not used in this pass
    uint tileSize;          // Not used in this pass
} pc;

void main()
{
    ivec2 tileCount = textureSize(uSourceImage, 0);
    ivec2 tile = ivec2(gl_FragCoord.xy);

    vec2 maxVelocity = vec2(0.0);
    float maxLengthSquared = 0.0;

    for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
            ivec2 coord = clamp(tile + ivec2(x, y), ivec2(0), tileCount - 1);
            vec2 velocity = texelFetch(uSourceImage, coord, 0).xy;
            float lengthSquared = dot(velocity, velocity);
            if (lengthSquared > maxLengthSquared) {
                maxLengthSquared = lengthSquared;
                maxVelocity = velocity;
            }
        }
    }

    outVelocity = vec4(maxVelocity, 0.0, 1.0);
}
