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

layout(push_constant) uniform ChromaticAberrationPushConstants {
    float exposure;                     // Manual exposure in EV
    uint pixelSize;                     // Size of the pixel blocks in screen pixels (1: No pixelation)
    uint colorLevels;                   // Color levels per channel (for example 4: 4 x 4 x 4 = 64 colors)
    uint isDitheringEnabled;            // 0: Plain posterization, 1: Ordered dithering (Bayer 8x8)
    uint isPostProcessingEffectEnabled; // 0: Effect disabled (only tone mapping), 1: Effect enabled
} pc;

// Samples per block axis for the block average (4 x 4 = 16 samples)
const int kBlockSampleCount = 4;

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

// Bayer 8x8 threshold in [0, 1): Recursive construction from the 2x2 matrix
float bayer8x8(uvec2 position)
{
    uint x = position.x & 7U;
    uint y = position.y & 7U;
    uint xy = x ^ y;

    // Bit interleaving of (x ^ y) and y gives the Bayer index
    uint index = ((xy & 1U) << 5) | ((y & 1U) << 4) | ((xy & 2U) << 2) | ((y & 2U) << 1) | ((xy & 4U) >> 1) | ((y & 4U) >> 2);

    return (float(index) + 0.5) / 64.0;
}

void main()
{
    vec2 resolution = vec2(textureSize(gLightPassOutput, 0));

    if (pc.isPostProcessingEffectEnabled == 0U) {
        vec3 hdrColor = texture(gLightPassOutput, gl_FragCoord.xy / resolution).rgb;
        outColor = vec4(tonemapAces(hdrColor * exp2(pc.exposure)), 1.0);
        return;
    }

    // Pixelation: All screen pixels in a block use the average color of the block, a single sample at the block center
    // would cause strong aliasing on detailed textures, so the block is averaged with a 4x4 grid of bilinear samples
    float pixelSize = float(max(pc.pixelSize, 1U));
    vec2 block = floor(gl_FragCoord.xy / pixelSize);
    vec2 blockStart = block * pixelSize;

    vec3 hdrColor = vec3(0.0);
    for (int y = 0; y < kBlockSampleCount; ++y) {
        for (int x = 0; x < kBlockSampleCount; ++x) {
            vec2 samplePosition = blockStart + (vec2(x, y) + 0.5) * (pixelSize / float(kBlockSampleCount));
            hdrColor += texture(gLightPassOutput, min(samplePosition, resolution - 0.5) / resolution).rgb;
        }
    }
    hdrColor /= float(kBlockSampleCount * kBlockSampleCount);

    // Exposure and tone mapping before the quantization (the color levels are display levels)
    vec3 displayColor = linearToSrgb(tonemapAces(hdrColor * exp2(pc.exposure)));

    // Posterization: "levels" values per channel in [0, 1], the step between two levels is "1 / (levels - 1)"
    float steps = float(max(pc.colorLevels, 2U) - 1U);

    // Ordered dithering: The threashold is added before rounding, so a color between two levels becomes a patern of
    // both levels (the average of the pattern is close to the original color), the pattern uses the block coordinates,
    // so the dither cells are as large as the pixel blocks
    float threshold = (pc.isDitheringEnabled != 0U) ? bayer8x8(uvec2(block)) - 0.5 : 0.0;
    vec3 quantizedColor = clamp(floor(displayColor * steps + 0.5 + threshold) / steps, 0.0, 1.0);

    outColor = vec4(srgbToLinear(quantizedColor), 1.0);
}
