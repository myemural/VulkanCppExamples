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

layout(push_constant) uniform ChromaticAberrationPushConstants {
    mat4 currentViewToPreviousClip; // previousProjection * previousView * inverse(currentView)
    float exposure;                 // Manual exposure in EV
    float blurScale;                // Shutter time / frame time (how much of the frame motion is captured)
    float maxBlurLength;            // Maximum blur length in pixels
    uint isMotionBlurEnabled;       // 0: Motion blur disabled (only tone mapping), 1: Motion blur enabled
    uint debugView;                 // 0: Final image, 1: Velocity view
} pc;

const uint DEBUG_VIEW_VELOCITY = 1U;

// Sample count along the velocity
const int kSampleCount = 16;

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

// Screen-space velocity (in pixels) of the pixel between the previous and the current frame
vec2 calculateVelocity(vec2 uv, vec2 resolution)
{
    vec4 positionView = texture(gPosition, uv);

    // No geometry (cleared G-Buffer), there is nothing to reporject
    if (positionView.w <= 0.0) {
        return vec2(0.0);
    }

    // Depth reprojection: Current view-space position to the clip-space of the previous frame, only the camera moves
    // in this scene, so the same world-space point is seen in the previous frame at this position
    vec4 previousClip = pc.currentViewToPreviousClip * vec4(positionView.xyz, 1.0);
    vec2 previousUv = (previousClip.xy / previousClip.w) * 0.5 + 0.5;

    // Velocity during the exposure time of the camera, limited to the maximum blur length
    vec2 velocity = (uv - previousUv) * resolution * pc.blurScale;
    float velocityLength = length(velocity);
    return velocityLength > pc.maxBlurLength ? velocity * (pc.maxBlurLength / velocityLength) : velocity;
}

void main()
{
    vec2 resolution = vec2(textureSize(gLightPassOutput, 0));
    vec2 uv = gl_FragCoord.xy / resolution;

    vec2 velocity = (pc.isMotionBlurEnabled != 0U || pc.debugView == DEBUG_VIEW_VELOCITY) ?
                    calculateVelocity(uv, resolution) : vec2(0.0);

    if (pc.debugView == DEBUG_VIEW_VELOCITY) {
        // Direction as color, length as brightness
        float strength = clamp(length(velocity) / pc.maxBlurLength, 0.0, 1.0);
        vec2 direction = length(velocity) > 1e-3 ? normalize(velocity) : vec2(0.0);
        outColor = vec4(vec3(0.5 + 0.5 * direction, 0.5) * strength, 1.0);
        return;
    }

    vec3 hdrColor = texture(gLightPassOutput, uv).rgb;

    // Samples are distributed along the velocity, centered on the pixel (the exposure is centered on the frame time)
    if (length(velocity) > 0.5) {
        vec2 velocityUv = velocity / resolution;
        float jitter = interleavedGradientNoise(gl_FragCoord.xy);

        // Shared sampler uses REPEAT address mode, so the coordinates are clamped to the image (half texel inside)
        vec2 minUv = 0.5 / resolution;
        vec2 maxUv = 1.0 - minUv;

        vec3 colorSum = vec3(0.0);
        for (int i = 0; i < kSampleCount; ++i) {
            float t = (float(i) + jitter) / float(kSampleCount) - 0.5;
            colorSum += texture(gLightPassOutput, clamp(uv + velocityUv * t, minUv, maxUv)).rgb;
        }
        hdrColor = colorSum / float(kSampleCount);
    }

    // Manual exposure control
    hdrColor *= exp2(pc.exposure);

    // Apply ACES tonemapping (HDR to LDR)
    outColor = vec4(tonemapAces(hdrColor), 1.0);
}
