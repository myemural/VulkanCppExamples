#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outVelocity;

layout(set = 0, binding = 0) uniform sampler2D uSourceImage; // Velocity buffer (UV units)

layout(push_constant) uniform VelocityTilePushConstants {
    float blurScale;        // Shutter time / frame time (how much of the frame motion is captured)
    float maxBlurRadius;    // Maximum blur radius in pixels (equal to the tile size)
    uint tileSize;          // Tile size in pixels
} pc;

// Velocity buffer (UV units, whole frame) to half-velocity in pixels during the exposure (blur radius)
vec2 toBlurVelocity(vec2 velocityUv, vec2 resolution)
{
    vec2 velocity = velocityUv * resolution * pc.blurScale * 0.5;
    float velocityLength = length(velocity);
    return velocityLength > pc.maxBlurRadius ? velocity * (pc.maxBlurRadius / velocityLength) : velocity;
}

void main()
{
    ivec2 resolution = textureSize(uSourceImage, 0);
    ivec2 tileStart = ivec2(gl_FragCoord.xy) * int(pc.tileSize);

    vec2 maxVelocity = vec2(0.0);
    float maxLengthSquared = 0.0;

    for(int y = 0; y < int(pc.tileSize); ++y) {
        for(int x = 0; x < int(pc.tileSize); ++x) {
            ivec2 coord = min(tileStart + ivec2(x, y), resolution - 1);
            vec2 velocity = toBlurVelocity(texelFetch(uSourceImage, coord, 0).xy, vec2(resolution));
            float lengthSquared = dot(velocity, velocity);
            if (lengthSquared > maxLengthSquared) {
                maxLengthSquared = lengthSquared;
                maxVelocity = velocity;
            }
        }
    }

    outVelocity = vec4(maxVelocity, 0.0, 1.0);
}
