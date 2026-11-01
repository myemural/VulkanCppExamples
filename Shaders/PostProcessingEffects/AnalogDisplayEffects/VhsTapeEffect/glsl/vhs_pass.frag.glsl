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
    float trackingError;     // 0: Perfect tracking, 1: Strong tracking band rolling over the screen
    float tapeNoise;         // 0: New tape, 1: Worn tape (more noise, jitter and dropouts)
    uint isVhsEnabled;       // 0: VHS disabled (only tone mapping), 1: VHS enabled
} pc;

// Video signal timing: NTSC draws ~60 fields per second with ~240 visible lines per field
const float kFieldRate = 59.94;
const float kLineCount = 240.0;

// Horizontal bandwidth of the VHS signal
const float kLumaSigma = 0.0012;
const float kChromaSigma = 0.0055;

// Chroma is recorded with a small delay, so the colors are shifted to the right of the edges
const float kChromaDelay = 0.0025;

// Horizontal sampling of the bandwidth filters
const int kTapRadius = 6;
const float kTapStep = 0.0015;

// Saturation of the played back color (VHS colors are slightly washed out)
const float kChromaGain = 0.85;

// Height of the head switching area at the bottom of the screen (in lines)
const float kHeadSwitchLines = 7.0;

// Vertical height and rolling speed of the tracking error band
const float kTrackingBandHeight = 0.09;
const float kTrackingBandSpeed = 0.06;

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

// Transforms from RGB to NTSC YIQ color space
vec3 rgbToYiq(vec3 rgb)
{
    return mat3(0.299, 0.596, 0.211,
                0.587, -0.274, -0.523,
                0.114, -0.322, 0.312) * rgb;
}

// Transforms from NTSC YIQ to RGB color space
vec3 yiqToRgb(vec3 yiq)
{
    return mat3(1.0, 1.0, 1.0,
                0.956, -0.272, -1.106,
                0.621, -0.647, 1.703) * yiq;
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

// Random value in [0, 1) for the given integer inputs
float hash(uint a, uint b, uint c)
{
    return float(pcg3d(uvec3(a, b, c)).x >> 8U) * (1.0 / 16777216.0);
}

// Smooth 1D value noise in [0, 1), used for slowly changing values
float valueNoise(float x, uint seed)
{
    float i = floor(x);
    float f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash(uint(int(i) + 10000), seed, 0U);
    float b = hash(uint(int(i) + 10001), seed, 0U);
    return mix(a, b, f);
}

// Display-encoded color of the scene
vec3 sampleScene(vec2 uv)
{
    if (uv.x < 0.0 || uv.x > 1.0) {
        return vec3(0.0);
    }

    vec3 hdrColor = texture(gLightPassOutput, vec2(uv.x, clamp(uv.y, 0.0, 1.0))).rgb;
    return linearToSrgb(tonemapAces(hdrColor * exp2(pc.exposure)));
}

// Intensity of the tracking error band at the vertical position Y
float trackingBand(float y)
{
    float bandCenter = fract(pc.time * kTrackingBandSpeed);
    float bandDistance = abs(y - bandCenter);
    bandDistance = min(bandDistance, 1.0 - bandDistance); // Wrap around the screen

    float flicker = 0.5 + 0.5 * valueNoise(pc.time * 3.0, 7U);
    return pc.trackingError * flicker * (1.0 - smoothstep(0.0, kTrackingBandHeight, bandDistance));
}

vec3 applyVhs(vec2 uv)
{
    uint field = uint(pc.time * kFieldRate);
    uint line = uint(clamp(uv.y, 0.0, 0.9999) * kLineCount);
    float wear = clamp(pc.tapeNoise, 0.0, 1.0);

    // 1) Horizontal displacement of the lines

    // Tape wobble: Stretching of the tape bends the image slowly
    float offset = 0.0012 * sin(uv.y * 7.0 + pc.time * 1.3) + 0.0006 * sin(uv.y * 23.0 - pc.time * 2.7);

    // Time base jitter: Every line starts at a slightly different time
    offset += (hash(line, field, 1U) - 0.5) * 0.0012 * (0.5 + wear);

    // Tracking error: Lines inside the band are torn horizontally
    float band = trackingBand(uv.y);
    offset += (hash(line, field, 2U) - 0.5) * 0.08 * band;

    // Head switching: The video heads switch a few lines before the vertical sync, the bottom lines are skewed
    float headSwitchStart = 1.0 - kHeadSwitchLines / kLineCount;
    float headSwitch = smoothstep(headSwitchStart, 1.0, uv.y);
    offset += headSwitch * headSwitch * (0.03 + 0.03 * hash(field, 0U, 3U));

    float x = uv.x + offset;

    // 2) Limited luma/chroma bandwidth (color bleeding)

    float lumaSum = 0.0;
    float lumaWeightSum = 0.0;
    vec2 chromaSum = vec2(0.0);
    float chromaWeightSum = 0.0;

    for (int i = -kTapRadius; i <= kTapRadius; ++i) {
        float tapOffset = float(i) * kTapStep;
        vec3 yiq = rgbToYiq(sampleScene(vec2(x + tapOffset, uv.y)));

        float lumaWeight = exp(-0.5 * (tapOffset * tapOffset) / (kLumaSigma * kLumaSigma));
        float chromaDistance = tapOffset + kChromaDelay; // Chroma filter is centered to the left (delayed signal)
        float chromaWeight = exp(-0.5 * (chromaDistance * chromaDistance) / (kChromaSigma * kChromaSigma));

        lumaSum += yiq.x * lumaWeight;
        lumaWeightSum += lumaWeight;
        chromaSum += yiq.yz * chromaWeight;
        chromaWeightSum += chromaWeight;
    }

    float luma = lumaSum / lumaWeightSum;
    vec2 chroma = chromaSum / chromaWeightSum * kChromaGain;

    // 3) Tape noise

    uvec2 pixel = uvec2(gl_FragCoord.xy);

    // Luma noise (fine, changes every field)
    luma += (hash(pixel.x, pixel.y, field) - 0.5) * 0.08 * wear;

    // Chroma noise: Horizontally stretched blotches, because chroma has a low bandwidth
    float chromaNoiseX = x * 90.0;
    chroma += (vec2(valueNoise(chromaNoiseX, line * 2U + field * 977U),
                    valueNoise(chromaNoiseX, line * 2U + 1U + field * 977U)) - 0.5) * 0.12 * wear;

    // Dropouts: Damaged parts of the tape produce short white streaks on random lines
    float dropoutChance = 0.004 * wear;
    if (hash(line, field, 4U) < dropoutChance) {
        float start = hash(line, field, 5U);
        float streakLength = 0.03 + 0.15 * hash(line, field, 6U);
        float streak = smoothstep(start, start + 0.01, x) *
                       (1.0 - smoothstep(start + streakLength - 0.03, start + streakLength, x));
        luma = mix(luma, 0.9 + 0.1 * hash(pixel.x, line, field), streak);
        chroma *= 1.0 - streak;
    }

    // Tracking band: Heavy noise and bright streaks inside the band
    float bandNoise = hash(pixel.x / 2U, line, field + 11U);
    luma = mix(luma, bandNoise * 0.9 + 0.1, band * step(0.6, hash(pixel.x / 24U, line, field + 12U)));
    chroma *= 1.0 - 0.7 * band;

    // Head switching area: Noisy, desaturated lines
    luma += (hash(pixel.x, pixel.y, field + 21U) - 0.5) * 0.5 * headSwitch;
    chroma *= 1.0 - headSwitch;

    // 4) Back to RGB

    vec3 displayColor = clamp(yiqToRgb(vec3(luma, chroma)), 0.0, 1.0);
    return srgbToLinear(displayColor);
}

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gLightPassOutput, 0));

    if (pc.isVhsEnabled == 0U) {
        // Only exposure and tone mapping
        vec3 hdrColor = texture(gLightPassOutput, uv).rgb;
        outColor = vec4(tonemapAces(hdrColor * exp2(pc.exposure)), 1.0);
        return;
    }

    outColor = vec4(applyVhs(uv), 1.0);
}
