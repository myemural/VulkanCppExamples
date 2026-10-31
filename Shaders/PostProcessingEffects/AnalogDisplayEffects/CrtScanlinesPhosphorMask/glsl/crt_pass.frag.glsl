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

layout(push_constant) uniform CrtPushConstants {
    float exposure;             // Manual exposure in EV
    float virtualLineCount;     // Number of scanlines of the virtual CRT image
    float scanlineStrength;     // 0: Wide overlapping beams (no visible gaps), 1: Thin beams with dark gaps
    uint maskType;              // 0: None, 1: Aperture Grills, 2: Slot Mask, 3: Shadow Mask
    uint isCrtEnabled;          // 0: CRT disabled (only tone mapping), 1: CRT enabled
} pc;

const uint MASK_TYPE_NONE = 0U;
const uint MASK_TYPE_APERTURE_GRILLE = 1U;
const uint MASK_TYPE_SLOT_MASK = 2U;
const uint MASK_TYPE_SHADOW_MASK = 3U;

// Brightness of the phosphors which don't belong to the current color channel
const float kMaskDark = 0.5;

// Horizontal width of the beam spot (in virtual pixels), small values give sharp pixels, large values a blurry image
const float kBeamWidthHorizontal = 0.4;

// Beam width (in line spacing units) for dark and bright colors
const float kBeamWidthWide = 0.5;
const float kBeamWidthThin = 0.16;

// Bright colors widen the beam (the electron beam gets stronger and spreads more on the phosphors)
const float kBeamBrightnessSpread = 0.6;

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

// Approximation of the error function (Winitzki) for positive values
vec3 erfApprox(vec3 x)
{
    const float a = 0.147;
    vec3 x2 = x * x;
    return sqrt(1.0 - exp(-x2 * (1.2732395 + a * x2) / (1.0 + a * x2)));
}

// Returns one pixel of the virtual low-res image, every virtual pixel covers ~3x3 source pixels
// So its area is box filtered with 4 bilinear taps
vec3 fetchVirtualPixel(ivec2 pixel, vec2 virtualResolution)
{
    pixel = clamp(pixel, ivec2(0), ivec2(virtualResolution) - 1);

    vec2 cellSize = 1.0 / virtualResolution;
    vec2 cellCenter = (vec2(pixel) + 0.5) * cellSize;

    // Main sampler repeats, so taps are kept inside the image
    vec2 halfTexel = 0.5 / vec2(textureSize(gLightPassOutput, 0));
    vec2 offsets[4] = vec2[](vec2(-0.25, -0.25), vec2(0.25, -0.25), vec2(-0.25, 0.25), vec2(0.25, 0.25));

    vec3 hdrColor = vec3(0.0);
    for (int i = 0; i < 4; ++i) {
        vec2 sampleUv = clamp(cellCenter + offsets[i] * cellSize, halfTexel, 1.0 - halfTexel);
        hdrColor += texture(gLightPassOutput, sampleUv).rgb;
    }
    hdrColor *= 0.25;

    // Beam simulation works on display referred (tone mapped) linear light
    return tonemapAces(hdrColor * exp2(pc.exposure));
}

// Color of a virtual line at the horizontal position X (in virtual pixels)
vec3 sampleLine(int line, float x, vec2 virtualResolution)
{
    int firstColumn = int(floor(x - 0.5)) - 1;

    vec3 color = vec3(0.0);
    float weightSum = 0.0;
    for (int i = 0; i < 4; ++i) {
        int column = firstColumn + i;
        float dx = x - (float(column) + 0.5);
        float weight = exp(-0.5 * (dx * dx) / (kBeamWidthHorizontal * kBeamWidthHorizontal));
        color += fetchVirtualPixel(ivec2(column, line), virtualResolution) * weight;
        weightSum += weight;
    }

    return color / weightSum;
}

// Vertical beam profile of a line, a gaussian whose width depends on the color (separately for every channel)
// Dark color give thin beams with visible gaps, bright colors give wide beams ehich almost fill the gaps
vec3 beamProfile(vec3 lineColor, float dy, float baseWidth)
{
    vec3 width = baseWidth * (1.0 + kBeamBrightnessSpread * lineColor);
    vec3 profile = exp(-0.5 * (dy * dy) / (width * width));

    // Normalization: Average of the gaussian over the two neighbouring line spacing is "width * sqrt(2 * PI) *
    // erf(1 / (width * sqrt(2))) / 2". Dividing by it keeps the average brightness of every line equal to its color,
    // so the scanlines only redistribute the light instead of darkening the image.
    vec3 average = width * 2.5066283 * erfApprox(1.0 / (width * 1.4142136)) * 0.5;
    return profile / average;
}

// Phosphor mask in screen pixel space, every mask lets one color channel through, the others are dimmed
vec3 phosphorMask(vec2 fragCoord, uint maskType)
{
    ivec2 p = ivec2(floor(fragCoord));
    vec3 mask = vec3(kMaskDark);

    if (maskType == MASK_TYPE_APERTURE_GRILLE) {
        // Vertical RGB stribes, one pixel per phosphor (3 pixels per triad)
        mask[p.x % 3] = 1.0;
    } else if (maskType == MASK_TYPE_SLOT_MASK) {
        // RGB slots of 3x4 pixels, every slot has a dark row at its bottom and neighbouring triads are sahifted by half
        // a slot, which gives the typical brick pattern of the consumer TVs
        int triad = p.x / 3;
        int row = (p.y + (triad % 2) * 2) % 4;
        if (row != 0) {
            mask[p.x % 3] = 1.0;
        }
    } else if (maskType == MASK_TYPE_SHADOW_MASK) {
        // Dot triads with 2-pixel wide phosphors
        mask[((p.x + p.y * 3) / 2) % 3] = 1.0;
    } else {
        mask = vec3(1.0);
    }

    return mask;
}

// Average brightness of a mask, used to compensate the light lost in the dimmed phosphors
float averageMaskBrightness(uint maskType)
{
    float triadAverage = (1.0 + 2.0 * kMaskDark) / 3.0;
    if (maskType == MASK_TYPE_APERTURE_GRILLE || maskType == MASK_TYPE_SHADOW_MASK) {
        return triadAverage;
    }
    if (maskType == MASK_TYPE_SLOT_MASK) {
        return 0.75 * triadAverage + 0.25 * kMaskDark; // 3/4 rows contain phosphors
    }

    return 1.0;
}

vec3 applyCrt(vec2 uv, vec2 fragCoord)
{
    vec2 screenSize = vec2(textureSize(gLightPassOutput, 0));
    float aspectRatio = screenSize.x / screenSize.y;
    vec2 virtualResolution = vec2(round(pc.virtualLineCount * aspectRatio), pc.virtualLineCount);

    // Position in the virtual image (in virtual pixels)
    vec2 virtualPos = uv * virtualResolution;

    // Every screen pixel lies between two virtual lines, both beams contribute to it
    int upperLine = int(floor(virtualPos.y - 0.5));
    int loweLine = upperLine + 1;
    float dyUpper = virtualPos.y - (float(upperLine) + 0.5);
    float dyLower = virtualPos.y - (float(loweLine) + 0.5);

    vec3 upperColor = sampleLine(upperLine, virtualPos.x, virtualResolution);
    vec3 lowerColor = sampleLine(loweLine, virtualPos.x, virtualResolution);

    float baseWidth = mix(kBeamWidthWide, kBeamWidthThin, clamp(pc.scanlineStrength, 0.0, 1.0));

    // Every beam is normalized over its support of two line spacing and every pixel gets two beams
    vec3 color = 0.5 * (upperColor * beamProfile(upperColor, dyUpper, baseWidth) +
                        lowerColor * beamProfile(lowerColor, dyLower, baseWidth));

    // Phosphor mask
    color *= phosphorMask(fragCoord, pc.maskType) / averageMaskBrightness(pc.maskType);

    return clamp(color, 0.0, 1.0);
}

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gLightPassOutput, 0));

    if (pc.isCrtEnabled == 0U) {
        // Only exposure and tone mapping
        vec3 hdrColor = texture(gLightPassOutput, uv).rgb;
        outColor = vec4(tonemapAces(hdrColor * exp2(pc.exposure)), 1.0);
        return;
    }

    outColor = vec4(applyCrt(uv, gl_FragCoord.xy), 1.0);
}
