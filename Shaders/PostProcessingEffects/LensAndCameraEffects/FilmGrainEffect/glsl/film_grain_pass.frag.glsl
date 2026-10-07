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

layout(push_constant) uniform FilmGrainPushConstants {
    float exposure;          // Manual exposure in EV
    float time;              // Animation time in seconds
    float grainIntensity;    // Strength of the grain in display space, range: [0.0, 0.5]
    float grainSize;         // Size of the grain cell in pixels, >= 1.0
    float grainFrameRate;    // How many times per second the grain pattern changes (24 = classic cinema)
    float luminanceResponse; // 0: Uniform grain over all tones, 1: Grain mostly on midtones
    uint colorGrain;         // 0: Monochrome grain, 1: Per-channel (colored) grain
    uint filmGrainEnabled;   // 0: Grain disabled (only tone mapping), 1: Grain enabled
} pc;

const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);

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

// Transforms from linear RGB color to sRGB color
vec3 linearToSrgb(vec3 c)
{
    vec3 lo = c * 12.92;
    vec3 hi = 1.055 * pow(max(c, vec3(1.0e-8)), vec3(1.0 / 2.4)) - 0.055;
    return mix(hi, lo, lessThanEqual(c, vec3(0.0031308)));
}

// Transforms from sRGB color to linear RGB color
vec3 srgbToLinear(vec3 c)
{
    vec3 lo = c / 12.92;
    vec3 hi = pow((c + 0.055) / 1.055, vec3(2.4));
    return mix(hi, lo, lessThanEqual(c, vec3(0.04045)));
}

// PCG 3D Hash, it gives three well-distributed 32-bit values from a 3D integer input
// Reference: https://jcgt.org/published/0009/03/02/ (Hash Functions for GPU Rendering)
uvec3 pcg3d(uvec3 v)
{
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.z;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v ^= v >> 16u;
    v.x += v.y * v.z;
    v.y += v.z * v.x;
    v.z += v.x * v.y;

    return v;
}

// Converts hashed integers to uniformly distributed floats in [0, 1)
vec3 hashToUnitFloat(uvec3 hash)
{
    return vec3(hash >> 8U) * (1.0 / 16777216.0);
}

// Zero-mean noise with a triangular distribution in [-1, 1] for a lattice point
// Sum of two uniform values gives a softer grain than a single uniform value
vec3 latticeNoise(ivec2 cell, uint frame)
{
    uvec3 seed = uvec3(uvec2(cell), frame);
    vec3 u0 = hashToUnitFloat(pcg3d(seed));
    vec3 u1 = hashToUnitFloat(pcg3d(seed ^ uvec3(0x9E3779B9U, 0x7F4A7C15U, 0x85EBCA6BU)));

    return u0 + u1 - 1.0;
}

// Value noise, which bilinearly interpolates the lattice noise
vec3 grainNoise(vec2 pixelCoord, float grainSize, uint frame)
{
    // Random offset for each frame, so that lattice lines don't stay at the same place on the screen
    vec2 frameOffset = hashToUnitFloat(pcg3d(uvec3(frame, 0x68E31DA4, 0xB5297A4DU))).xy * 1024.0;

    vec2 p = pixelCoord / max(grainSize, 1.0) + frameOffset;
    ivec2 i = ivec2(floor(p));
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f); // Smoothstep interpolation

    vec3 n00 =latticeNoise(i, frame);
    vec3 n01 =latticeNoise(i + ivec2(0, 1), frame);
    vec3 n10 =latticeNoise(i + ivec2(1, 0), frame);
    vec3 n11 =latticeNoise(i + ivec2(1, 1), frame);

    return mix(mix(n00, n10, f.x), mix(n01, n11, f.x), f.y);
}

float convertToLuminance(vec3 color)
{
    return dot(color, kLumaRec709);
}

vec3 applyFilmGrain(vec3 displayColor, vec2 pixelCoord)
{
    // Quantize time, so the grain pattern changes with the "film" frame rate intead of the render frame rate
    uint frame = uint(float(pc.time * max(pc.grainFrameRate, 1.0)));

    vec3 noise = grainNoise(pixelCoord, pc.grainSize, frame);
    if (pc.colorGrain == 0U) {
        noise = vec3(noise.x); // Same noise value for all channels to provide monochrome grain
    }

    // Film grain is the most viisble on midtones
    // "4 * L * (1 - L)" is 1.0 on mid-grey and 0.0 on pure black/white
    float luma = clamp(convertToLuminance(displayColor), 0.0, 1.0);
    float midtoneMask = 4.0 * luma * (1.0 - luma);
    float response = mix(1.0, midtoneMask, clamp(pc.luminanceResponse, 0.0, 1.0));

    return clamp(displayColor + noise * pc.grainIntensity * response, 0.0, 1.0);
}

void main()
{
    // Sampling the HDR output of the light pass
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gLightPassOutput, 0));
    vec3 hdrColor = texture(gLightPassOutput, uv).xyz;

    // Manual exposure control
    vec3 exposedColor = hdrColor * exp2(pc.exposure);

    // Apply ACES tonemapping (HDR to LDR)
    vec3 ldrColor = tonemapAces(exposedColor);

    if (pc.filmGrainEnabled != 0U) {
        vec3 displayColor = linearToSrgb(ldrColor);
        displayColor = applyFilmGrain(displayColor, gl_FragCoord.xy);
        ldrColor = srgbToLinear(displayColor);
    }

    outColor = vec4(ldrColor, 1.0);
}
