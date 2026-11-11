#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D gLightPassOutput;
layout(set = 0, binding = 1) uniform sampler2D gPosition;
layout(set = 0, binding = 2) uniform sampler2D gVelocity; // Screen-space velocity (UV units)
layout(set = 0, binding = 3) uniform sampler2D uNeighborMaxImage; // Dominant half-velocity of the neighborhood

layout(push_constant) uniform ChromaticAberrationPushConstants {
    float exposure;                 // Manual exposure in EV
    float blurScale;                // Shutter time / frame time (how much of the frame motion is captured)
    float maxBlurRadius;            // Maximum blur radius in pixels (equal to the tile size)
    uint isMotionBlurEnabled;       // 0: Motion blur disabled (only tone mapping), 1: Motion blur enabled
    uint debugView;                 // 0: Final image, 1: Velocity buffer, 2: NeighborMax tiles
} pc;

const uint DEBUG_VIEW_VELOCITY = 1U;
const uint DEBUG_VIEW_NEIGHBOR_MAX = 2U;

// Sample count along the dominant velocity (odd, the center is the pixel itself)
const int kSampleCount = 15;

// Depth difference (in meters) where the soft depth comparison changes from "in front" to "behind"
const float kSoftDepthExtent = 0.2;

// Pixels without geometry (cleared G-Buffer) are treated as very far away
const float kBackgroundDepth = 10000.0;

// ACES Filmic (Narkowicz fit)
// Reference: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 tonemapAces(vec3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

// Interleaved gradient noise (Jimenez 2014)
float interleavedGradientNoise(vec2 p)
{
    return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

// Velocity buffer (UV units, whole frame) to half-velocity in pixels during the exposure (blur radius)
vec2 readBlurVelocity(vec2 uv, vec2 resolution)
{
    vec2 velocity = texture(gVelocity, uv).xy * resolution * pc.blurScale * 0.5;
    float velocityLength = length(velocity);
    return velocityLength > pc.maxBlurRadius ? velocity * (pc.maxBlurRadius / velocityLength) : velocity;
}

// Tiles are read without filtering (the velocity of a tile is not interpolated with its neighbors)
vec2 readNeighborMax(vec2 uv)
{
    ivec2 tileCount = textureSize(uNeighborMaxImage, 0);
    ivec2 tile = min(ivec2(uv * vec2(tileCount)), tileCount - 1);
    return texelFetch(uNeighborMaxImage, tile, 0).xy;
}

float readDepth(vec2 uv)
{
    vec4 positionView = texture(gPosition, uv);
    return positionView.w > 0.0 ? -positionView.z : kBackgroundDepth;
}

// 1.0 if depth A is in front of depth B (with a soft transition)
float softDepthInFront(float depthA, float depthB)
{
    return clamp(1.0 - (depthA - depthB) / kSoftDepthExtent, 0.0, 1.0);
}

// A pixel with blur radius "radius" covers the points closer than its readius (linear falloff)
float cone(float distance, float radius)
{
    return clamp(1.0 - distance / radius, 0.0, 1.0);
}

// Same as the cone, but with a flat profile (used when both pixels are blurred)
float cylinder(float distance, float radius)
{
    return 1.0 - smoothstep(0.95 * radius, 1.05 * radius, distance);
}

vec3 reconstructMotionBlur(vec2 uv, vec2 resolution)
{
    vec3 centerColor = texture(gLightPassOutput, uv).rgb;

    // Dominant velocity of the neighborhood, nothing to blur if it is smaller than half pixel
    vec2 neighborMax = readNeighborMax(uv);
    float neighborMaxLength = length(neighborMax);
    if (neighborMaxLength <= 0.5) {
        return centerColor;
    }

    float centerRadius = max(length(readBlurVelocity(uv, resolution)), 0.5);
    float centerDepth = readDepth(uv);

    // The center pixel's weight is ineversely proportional to its blur radius (its color spreads over a longer line)
    float weightSum = 1.0 / centerRadius;
    vec3 colorSum = centerColor * weightSum;

    // Shared sampler uses REPEAT address mode, so the coordinates are clamped to the image (half texel inside)
    vec2 minUv = 0.5 / resolution;
    vec2 maxUv = 1.0 - minUv;

    float jitter = interleavedGradientNoise(gl_FragCoord.xy) - 0.5;
    for (int i = 0; i < kSampleCount; ++i) {
        if (i == kSampleCount / 2) {
            continue; // Cemnter pixel is already added
        }

        // Centers along the dominant velocity in [-1, 1] (both directions, the exposure is centered on the frame)
        float t = mix(-1.0, 1.0, (float(i) + jitter + 1.0) / float(kSampleCount + 1));
        vec2 sampleUv = clamp(uv + neighborMax * t / resolution, minUv, maxUv);

        float sampleDistance = abs(t) * neighborMaxLength;
        float sampleRadius = max(length(readBlurVelocity(sampleUv, resolution)), 0.5);
        float sampleDepth = readDepth(sampleUv);

        float sampleInFront = softDepthInFront(sampleDepth, centerDepth);
        float sampleBehind = softDepthInFront(centerDepth, sampleDepth);

        // Case 1: Blurred sample in front of the center, its blur covers the center
        float weight = sampleInFront * cone(sampleDistance, sampleRadius);
        // Case 2: Blurred center in front of the sample, the background (sample) is seen through the center's blur
        weight += sampleBehind * cone(sampleDistance, centerRadius);
        // Case 3: Both are blurred (for example, the same moving object), they blur into each other
        weight += cylinder(sampleDistance, sampleRadius) * cylinder(sampleDistance, centerRadius) * 2.0;

        colorSum += texture(gLightPassOutput, sampleUv).rgb * weight;
        weightSum += weight;
    }

    return colorSum / weightSum;
}

// Direction as color, length as brightness
vec3 visualizeVelocity(vec2 velocity)
{
    float strength = clamp(length(velocity) / pc.maxBlurRadius, 0.0, 1.0);
    vec2 direction = length(velocity) > 1e-3 ? normalize(velocity) : vec2(0.0);
    return vec3(0.5 + 0.5 * direction, 0.5) * strength;
}

void main()
{
    vec2 resolution = vec2(textureSize(gLightPassOutput, 0));
    vec2 uv = gl_FragCoord.xy / resolution;

    if (pc.debugView == DEBUG_VIEW_VELOCITY) {
        outColor = vec4(visualizeVelocity(readBlurVelocity(uv, resolution)), 1.0);
        return;
    }
    if (pc.debugView == DEBUG_VIEW_NEIGHBOR_MAX) {
        outColor = vec4(visualizeVelocity(readNeighborMax(uv)), 1.0);
        return;
    }

    vec3 hdrColor = (pc.isMotionBlurEnabled != 0U) ? reconstructMotionBlur(uv, resolution) :
                                                     texture(gLightPassOutput, uv).rgb;

    // Manual exposure control
    hdrColor *= exp2(pc.exposure);

    // Apply ACES tonemapping (HDR to LDR)
    outColor = vec4(tonemapAces(hdrColor), 1.0);
}
