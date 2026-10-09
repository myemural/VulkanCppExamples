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
    float intensity;                    // Maximum channel offset at the screen corners (in pixels, defined for 720p)
    float falloff;                      // Exponent of the radial falloff (1: Linear, >1: effect concentrates on edges)
    uint isChromaticAberrationEnabled;  // 0: Effect disabled (only tone mapping), 1: Effect enabled
} pc;

const float kReferenceHeight = 720.0;

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

// Lateral (transverse) chromatic aberration: A real lens has a slightly different magnification for ecah wavelength
// So, the color channels are shifted radially by an amount growing with the distance
// Red is sampled closer to the center (appears pushed outwards), blue is sampled farther (appears pulled inwards)
vec3 sampleWithChromaticAberration(vec2 uv, vec2 resolution)
{
    // Direction from the screen center in pixel space
    vec2 fromCenterPx = (uv - 0.5) * resolution;
    float distancePx = length(fromCenterPx);
    if (distancePx < 1e-3) {
        return texture(gLightPassOutput, uv).xyz;
    }

    // Normalized distance, 0.0 at the center, 1.0 at the screen corners
    float maxDistancePx = 0.5 * length(resolution);
    float normalizedDistance = distancePx / maxDistancePx;

    // Offset is defined for 720p and scaled with the resolution, so the effect looks the same in every resolution
    float offsetPx = pc.intensity * (resolution.y / kReferenceHeight) * pow(normalizedDistance, pc.falloff);
    vec2 offsetUv = (fromCenterPx / distancePx) * offsetPx / resolution;

    // Shared sampler uses REPEAT address mode, so the shifted coordinates are clamped to the image (half texel inside)
    // This is required for prevent sampling from the opposite side of the screen on the edges
    vec2 minUv = 0.5 / resolution;
    vec2 maxUv = 1.0 - minUv;

    float r = texture(gLightPassOutput, clamp(uv - offsetUv, minUv, maxUv)).r;
    float g = texture(gLightPassOutput, uv).g;
    float b = texture(gLightPassOutput, clamp(uv + offsetUv, minUv, maxUv)).b;

    return vec3(r, g, b);
}

void main()
{
    vec2 resolution = vec2(textureSize(gLightPassOutput, 0));
    vec2 uv = gl_FragCoord.xy / resolution;

    // Chromatic aberration is applied while sampling the HDR output of the light pass
    // It is like a real lens which acts on the incoming light
    vec3 hdrColor = (pc.isChromaticAberrationEnabled != 0U) ? sampleWithChromaticAberration(uv, resolution) :
                                                              texture(gLightPassOutput, uv).xyz;

    // Manual exposure control
    vec3 exposedColor = hdrColor * exp2(pc.exposure);

    // Apply ACES tonemapping (HDR to LDR)
    outColor = vec4(tonemapAces(exposedColor), 1.0);
}
