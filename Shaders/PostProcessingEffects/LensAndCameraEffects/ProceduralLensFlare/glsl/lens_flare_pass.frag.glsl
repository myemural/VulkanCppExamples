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

struct PointLightData
{
    vec4 lightPosition;       // xyz = Light Position (View-Space)
    vec4 lightColorIntensity; // xyz = Light Color, w = Color Intensity
};

layout(std430, set = 0, binding = 2) readonly buffer PointLightBuffer
{
    PointLightData lights[];
};

layout(push_constant) uniform VignettePushConstants {
    mat4 projection;         // Projection matrix of the camera (for projecting lights onto the screen)
    float exposure;          // Manual exposure in EV
    float flareIntensity;    // Global multiplier for all flare elements
    float lightSphereRadius; // Radius of the light spheres (used for occlusion test)
    uint lightCount;         // Number of point lights
    uint flareElementMask;   // Enabled flare elements (bit flags)
    uint isLensFlareEnabled; // 0: Lens flare disabled (only tone mapping), 1: Lens flare enabled
} pc;

// Flare element bit flags
const uint FLARE_GLOW = 1U << 0U;
const uint FLARE_STARBURST = 1U << 1U;
const uint FLARE_STREAK = 1U << 2U;
const uint FLARE_GHOSTS = 1U << 3U;
const uint FLARE_HALO = 1U << 4U;

// Exponent for the flare brightness (1.0 is physically linear)
const float kFlareResponse = 0.6;

// Visibility test sample grid size ("kVisibilityGridSize * kVisibilityGridSize" samples, only the ones inside a disk)
const int kVisibilityGridSize = 5;

const float kPi = 3.14159265;

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

// Converts UV to the centered and aspect corrected flare space ((0, 0) is the screen center, screen height is 1.0)
vec2 toFlareSpace(vec2 uv, float aspectRatio)
{
    return (uv - 0.5) * vec2(aspectRatio, 1.0);
}

// Returns the visible fraction [0, 1] of the light sphere
// Samples the view-space positions in the G-buffer over the projected disk of the sphere
// And count the samples which are not occluded by a closer surface
// Samples outside of the screen are counted as occluded, so the flare fades out smoothly at the screen edges
float calculateLightVisibility(vec2 lightUv, float lightDepth, float projectedRadiusUv, vec2 resolution)
{
    float aspectRatio = resolution.x / resolution.y;

    // At least a few pixels, so very far lights still have a stable (non-flickering) visibility
    float radiusUv = max(projectedRadiusUv, 2.0 / resolution.y);

    // Anything closer than the front surface of the sphere (with a small tolerance) occludes the light
    float occluderDepth = lightDepth - pc.lightSphereRadius * 1.5;

    float visibleCount = 0.0;
    float totalCount = 0.0;
    for (int y = 0; y < kVisibilityGridSize; ++y) {
        for (int x = 0; x < kVisibilityGridSize; ++x) {
            // Grid point in [-1, 1], only the ones inside of the unit disk are used
            vec2 gridPoint = (vec2(x, y) + 0.5) / float(kVisibilityGridSize) * 2.0 - 1.0;
            if (dot(gridPoint, gridPoint) > 1.0) {
                continue;
            }
            totalCount += 1.0;

            vec2 sampleUv = lightUv + gridPoint * vec2(radiusUv / aspectRatio, radiusUv);
            if (any(lessThan(sampleUv, vec2(0.0))) || any(greaterThan(sampleUv, vec2(1.0)))) {
                continue;
            }

            // texelFetch is used, because linear filtering mixes positions of different surfaces on the edges
            vec4 scenePosition = texelFetch(gPosition, ivec2(sampleUv * resolution), 0);

            // w = 0.0 means there is no geometry (cleared background), so nothing occludes the light there
            bool isBackground = scenePosition.w < 0.5;
            if (isBackground || -scenePosition.z >= occluderDepth) {
                visibleCount += 1.0;
            }
        }
    }

    return visibleCount / max(totalCount, 1.0);
}

// Bright core of the light with a soft wide tail (scattering inside the lens)
float flareGlow(vec2 toPixel)
{
    float d = length(toPixel);
    return 0.5 * exp(-d / 0.02) + 0.12 * exp(-d / 0.12);
}

// Starburst rays caused by diffraction on the aperture blades (6 blades gives 6 rays)
float flareStarburst(vec2 toPixel)
{
    const float kBladeCount = 6.0;
    float d = length(toPixel);
    float angle = atan(toPixel.y, toPixel.x);

    float mainRays = pow(abs(cos(angle * kBladeCount * 0.5)), 80.0);
    float secondaryRays = pow(abs(cos(angle * kBladeCount * 0.5 + kPi / kBladeCount)), 200.0) * 0.3;

    return (mainRays + secondaryRays) * exp(-d / 0.09) * 0.25;
}

// Horizontal anamorphic streak (cinematic anamorphic lenses stretch the flare horizontally)
float flareStreak(vec2 toPixel)
{
    return exp(-abs(toPixel.y) / 0.004) * exp(-abs(toPixel.x) / 0.5) * 0.25;
}

// Ghosts are the reflections between lens elements, they lay on the line which passes through the light position and
// the screen center, so their position is a scale of the light position (negative scale means the opposite side)
vec3 flareGhosts(vec2 pixel, vec2 lightPos)
{
    // x: Position scale along the light-center line, y: Radius, z: Intensity
    const vec3 kGhosts[5] = vec3[5](
        vec3(-0.25, 0.025, 0.1),
        vec3(-0.55, 0.06, 0.05),
        vec3(-0.95, 0.11, 0.03),
        vec3(-1.4, 0.045, 0.06),
        vec3(0.45, 0.018, 0.08)
    );

    // Each ghost has a slightly different tint (lens coatings reflect different wavelengths)
    const vec3 kGhostTints[5] = vec3[5](
            vec3(1.0, 0.8, 0.4),
            vec3(0.4, 1.0, 0.6),
            vec3(0.5, 0.6, 1.0),
            vec3(1.0, 0.5, 0.8),
            vec3(0.8, 0.9, 1.0)
    );

    vec3 result = vec3(0.0);
    for (int i = 0; i < 5; ++i) {
        vec2 ghostCenter = lightPos * kGhosts[i].x;
        float radius = kGhosts[i].y;
        float d = length(pixel - ghostCenter);

        // Soft disk with a slightly brighter rim
        float disk = smoothstep(radius, radius * 0.6, d);
        float rim = smoothstep(radius * 0.15, 0.0, abs(d - radius * 0.85)) * 0.5;

        result += (disk + rim) * kGhosts[i].z * kGhostTints[i];
    }

    return result;
}

// Halo is a ring around the screen center, which becomes visible on the side facing the light
// Each color channel uses a slightly different radius, so the ring has a rainbow-like (chromatic) look
vec3 flareHalo(vec2 pixel, vec2 lightPos)
{
    const float kHaloRadius = 0.45;
    const float kHaloWidth = 0.025;
    const vec3 kChannelRadiusScale = vec3(0.97, 1.0, 1.03);

    float lightDistanceToCenter = length(lightPos);
    float pixelDistanceToCenter = length(pixel);
    if (lightDistanceToCenter < 1e-4 || pixelDistanceToCenter < 1e-4) {
        return vec3(0.0);
    }

    // Only the part of the ring on the light side is visible
    float facing = pow(max(dot(pixel / pixelDistanceToCenter, lightPos / lightDistanceToCenter), 0.0), 4.0);
    facing *= smoothstep(0.0, 0.15, lightDistanceToCenter);

    vec3 ringDistance = (vec3(pixelDistanceToCenter) - kHaloRadius * kChannelRadiusScale) / kHaloWidth;
    vec3 ring = exp(-ringDistance * ringDistance);

    return ring * facing * 0.08;
}

// Calculates all enabled flare elements of a light for the current pixel
vec3 calculateFlare(vec2 pixel, vec2 lightPos, vec3 lightColor)
{
    vec2 toPixel = pixel - lightPos;
    vec3 flare = vec3(0.0);

    if ((pc.flareElementMask & FLARE_GLOW) != 0U) {
        flare += flareGlow(toPixel) * lightColor;
    }
    if ((pc.flareElementMask & FLARE_STARBURST) != 0U) {
        flare += flareStarburst(toPixel) * lightColor;
    }
    if ((pc.flareElementMask & FLARE_STREAK) != 0U) {
        // Anamorphic streaks are usually bluish because of the lens coatings
        flare += flareStreak(toPixel) * mix(lightColor, vec3(0.4, 0.6, 1.0), 0.6);
    }
    if ((pc.flareElementMask & FLARE_GHOSTS) != 0U) {
        flare += flareGhosts(pixel, lightPos) * lightColor;
    }
    if ((pc.flareElementMask & FLARE_HALO) != 0U) {
        flare += flareHalo(pixel, lightPos) * lightColor;
    }

    return flare;
}

vec3 calculateLensFlare(vec2 uv, vec2 resolution)
{
    float aspectRatio = resolution.x / resolution.y;
    vec2 pixel = toFlareSpace(uv, aspectRatio);

    vec3 result = vec3(0.0);
    for (uint i = 0U; i < pc.lightCount; ++i) {
        vec3 lightPosView = lights[i].lightPosition.xyz;
        float lightDepth = -lightPosView.z;

        // Lights behind the camera (or too close to it) don't produce a flare
        if (lightDepth <= pc.lightSphereRadius) {
            continue;
        }

        // Project the light onto the screen
        vec4 clipPos = pc.projection * vec4(lightPosView, 1.0);
        vec2 lightUv = (clipPos.xy / clipPos.w) * 0.5 + 0.5;

        // Projected sphere radius in UV (screen height) units
        float projectedRadiusUv = abs(pc.projection[1][1]) * pc.lightSphereRadius / lightDepth * 0.5;

        float visibility = calculateLightVisibility(lightUv, lightDepth, projectedRadiusUv, resolution);
        if (visibility <= 0.0) {
            continue;
        }

        // Irradiance arriving to the lens from the light, compressed by the flare response
        float lightIntensity = lights[i].lightColorIntensity.a;
        float distanceSquared = max(dot(lightPosView, lightPosView), 1.0);
        float brightness = pow(lightIntensity / distanceSquared, kFlareResponse);

        vec2 lightPos = toFlareSpace(lightUv, aspectRatio);
        vec3 lightColor = lights[i].lightColorIntensity.rgb;

        result += calculateFlare(pixel, lightPos, lightColor) * brightness * visibility;
    }

    return result * pc.flareIntensity;
}

void main()
{
    vec2 resolution = vec2(textureSize(gLightPassOutput, 0));
    vec2 uv = gl_FragCoord.xy / resolution;

    // Sampling the HDR output of the light pass
    vec3 hdrColor = texture(gLightPassOutput, uv).xyz;

    // Lens flare is added in HDR space before exposure and tone mapping, because it is an optical effect of the lens
    if (pc.isLensFlareEnabled != 0U) {
        hdrColor += calculateLensFlare(uv, resolution);
    }

    // Manual exposure control
    vec3 exposedColor = hdrColor * exp2(pc.exposure);

    // Apply ACES tonemapping (HDR to LDR)
    outColor = vec4(tonemapAces(exposedColor), 1.0);
}
