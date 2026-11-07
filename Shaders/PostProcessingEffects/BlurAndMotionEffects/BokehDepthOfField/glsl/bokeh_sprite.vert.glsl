#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec2 outLocalPosition; // Position relative to the sprite center (in half-res pixels)
layout(location = 1) flat out vec3 outColor; // Sprite color (energy divided by the aperture area)
layout(location = 2) flat out float outRadius; // Sprite radius (CoC radius in half-res pixels)
layout(location = 3) flat out float outCoc; // Signed CoC of the sprite

layout(set = 0, binding = 0) uniform sampler2D uBokehSourceImage; // rgb = Bokeh Color, a = Signed CoC

layout(push_constant) uniform BokehPassPushConstants {
    vec2 texelSize;   // 1.0 / half-res image size
    uint bladeCount;  // Aperture blade count (0: Circular)
    float bladeAngle; // Rotation of the aperture polygon in radians
} pc;

const float PI = 3.14159265359;
const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);

// Sprites with very small CoC or energy are skipped
const float kMinSpriteRadius = 1.5;
const float kMinSpriteLuminance = 1e-3;

// Area of the aperture shape with radius 1 (circle or regular polygon)
float apertureArea(uint bladeCount)
{
    if (bladeCount < 3U) {
        return PI;
    }

    float n = float(bladeCount);
    return 0.5 * n * sin(2.0 * PI / n);
}

void main()
{
    const vec2 corners[6] = vec2[](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0),
                                   vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, 1.0));

    // Instance index to the pixel coordinate of the half-res image
    ivec2 imageSize = textureSize(uBokehSourceImage, 0);
    ivec2 pixel = ivec2(gl_InstanceIndex % imageSize.x, gl_InstanceIndex / imageSize.x);
    vec4 source = texelFetch(uBokehSourceImage, pixel, 0);

    float radius = abs(source.a);
    float luma = dot(source.rgb, kLumaRec709);
    if (radius < kMinSpriteRadius || luma < kMinSpriteLuminance) {
        // Degenerate quad outside of the clip-space, it is culled before rasterization
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        return;
    }

    // Quad is a little bigger than the sprite for anti-aliased edges
    vec2 localPosition = corners[gl_VertexIndex] * (radius + 1.0);
    vec2 screenPosition = (vec2(pixel) + 0.5 + localPosition) * pc.texelSize;

    outLocalPosition = localPosition;
    outRadius = radius;
    outCoc = source.a;

    // Energy of the pixel is distributed over the aperture shape, so the total energy doesn't change
    outColor = source.rgb / (apertureArea(pc.bladeCount) * radius * radius);

    gl_Position = vec4(screenPosition * 2.0 - 1.0, 0.0, 1.0);
}
