#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D uVirtualImage; // 480 lines, tone mapped linear color
layout(set = 0, binding = 1) uniform sampler2D uHalationImage; // Blurred virtual image
layout(set = 0, binding = 2) uniform sampler2D gLightPassOutput; // Only used when the CRT effect is disabled

layout(push_constant) uniform CrtPushConstants {
    float exposure;             // Manual exposure in EV (only used when the CRT effect is disabled)
    float lineCount;            // Number of scanlines (240: Progressive, 480: Interlaced)
    float curvature;            // Screen curvature amount (0: Flat screen)
    float halationStrength;     // Amount of the glow scattered in the glass of the tube
    uint isCrtEnabled;          // 0: CRT disabled (only tone mapping), 1: CRT enabled
} pc;

// Curvature of the tube for curvature = 1 (horizontal, vertical)
const vec2 kCurvature = vec2(1.0 / 32.0, 1.0 / 24.0);

// Rounded corners of the tube
const float kCornerRadius = 0.06;
const float kEdgeSoftness = 2.0;

// Scanline and mask settings
const float kScanlineStrength = 0.6;
const float kBeamWidthHorizontal = 0.4;
const float kBeamWidthWide = 0.5;
const float kBeamWidthThin = 0.16;
const float kBeamBrightnessSpread = 0.6;
const float kMaskDark = 0.5;

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

// Curved tube, screen position is pushed outwards, more on the edges than in the center
vec2 warpScreen(vec2 uv, float amount)
{
    vec2 pos = uv * 2.0 - 1.0;
    pos *= vec2(1.0 + pos.y * pos.y * kCurvature.x * amount, 1.0 + pos.x * pos.x * kCurvature.y * amount);
    return pos * 0.5 + 0.5;
}

// Rounded rectangle signed distance in pixels gives smooth corners
// 1: Inside the visible area of the tube, 0: Outside
float tubeMask(vec2 warpedUv, vec2 screenSize)
{
    vec2 halfSize = 0.5 * screenSize;
    float radius = kCornerRadius * screenSize.y;
    vec2 p = abs((warpedUv - 0.5) * screenSize);
    float signedDistance = length(max(p - (halfSize - radius), 0.0)) - radius;
    return clamp(-signedDistance / kEdgeSoftness, 0.0, 1.0);
}

// One pixel of a scanline
vec3 fetchLinePixel(int column, int line, ivec2 virtualSize)
{
    int rowsPerLine = virtualSize.y / int(pc.lineCount);
    column = clamp(column, 0, virtualSize.x - 1);
    line = clamp(line, 0, int(pc.lineCount) - 1);

    vec3 color = vec3(0.0);
    for (int i = 0; i < rowsPerLine; ++i) {
        color += texelFetch(uVirtualImage, ivec2(column, line * rowsPerLine + i), 0).rgb;
    }

    return color / float(rowsPerLine);
}

// Color of a scanline at the horizontal position X (in virtual pixels)
vec3 sampleLine(int line, float x, ivec2 virtualSize)
{
    int firstColumn = int(floor(x - 0.5)) - 1;

    vec3 color = vec3(0.0);
    float weightSum = 0.0;
    for (int i = 0; i < 4; ++i) {
        int column = firstColumn + i;
        float dx = x - (float(column) + 0.5);
        float weight = exp(-0.5 * (dx * dx) / (kBeamWidthHorizontal * kBeamWidthHorizontal));
        color += fetchLinePixel(column, line, virtualSize) * weight;
        weightSum += weight;
    }

    return color / weightSum;
}

// Brightness dependent gaussian beam profile, normalized to keep the average brightness of the line
vec3 beamProfile(vec3 lineColor, float dy, float baseWidth)
{
    vec3 width = baseWidth * (1.0 + kBeamBrightnessSpread * lineColor);
    vec3 profile = exp(-0.5 * (dy * dy) / (width * width));
    vec3 average = width * 2.5066283 * erfApprox(1.0 / (width * 1.4142136)) * 0.5;
    return profile / average;
}

// Aperture grille in screen pixel space with brightness compensation
vec3 apertureGrille(vec2 fragCoord)
{
    vec3 mask = vec3(kMaskDark);
    mask[int(floor(fragCoord.x)) % 3] = 1.0;
    return mask * (3.0 / (1.0 + 2.0 * kMaskDark));
}

vec3 applyCrt(vec2 warpedUv, vec2 fragCoord)
{
    ivec2 virtualSize = textureSize(uVirtualImage, 0);
    vec2 virtualPos = warpedUv * vec2(float(virtualSize.x), pc.lineCount);

    // Two neighbouring beams contribute to every screen pixel
    int upperLine = int(floor(virtualPos.y - 0.5));
    int loweLine = upperLine + 1;
    float dyUpper = virtualPos.y - (float(upperLine) + 0.5);
    float dyLower = virtualPos.y - (float(loweLine) + 0.5);

    vec3 upperColor = sampleLine(upperLine, virtualPos.x, virtualSize);
    vec3 lowerColor = sampleLine(loweLine, virtualPos.x, virtualSize);

    float baseWidth = mix(kBeamWidthWide, kBeamWidthThin, kScanlineStrength);
    vec3 color = 0.5 * (upperColor * beamProfile(upperColor, dyUpper, baseWidth) +
                        lowerColor * beamProfile(lowerColor, dyLower, baseWidth));

    // Phosphor mask
    color *= apertureGrille(fragCoord);

    // Halation: Light of the phosphors scatters in the thick glass of the tube and comes back as a wide, soft glow
    color += texture(uHalationImage, warpedUv).rgb * pc.halationStrength;

    return color;
}

void main()
{
    vec2 screenSize = vec2(textureSize(gLightPassOutput, 0));
    vec2 uv = gl_FragCoord.xy / screenSize;

    if (pc.isCrtEnabled == 0U) {
        // Only exposure and tone mapping
        vec3 hdrColor = texture(gLightPassOutput, uv).rgb;
        outColor = vec4(tonemapAces(hdrColor * exp2(pc.exposure)), 1.0);
        return;
    }

    vec2 warpedUv = warpScreen(uv, pc.curvature);
    float mask = tubeMask(warpedUv, screenSize);
    if (mask <= 0.0) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0); // Outside of the tube
        return;
    }

    outColor = vec4(clamp(applyCrt(warpedUv, gl_FragCoord.xy) * mask, 0.0, 1.0), 1.0);
}
