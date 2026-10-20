#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define TWO_PI 6.28318530718

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D gSceneColor;
layout(set = 0, binding = 1) uniform sampler2D gSssIrradiance; // rgb = Diffuse irradiance, a = View depth (0 = none)
layout(set = 0, binding = 2) uniform sampler2D gSssAlbedo;     // rgb = Albedo
layout(set = 0, binding = 3) uniform sampler2D gSssScatter;    // rgb = Scatter distance d per channel

layout(push_constant) uniform CompositePushConstants {
    vec2 uvPerUnit; // UV offset per world unit at view depth 1 (x: horizontal, y: vertical)
} pc;

// Number of profile samples per channel, must be a multiple of 4 so that both lobes get equal weight
const int kSssSamplesPerChannel = 16;
const int kSssFirstLobeCount = kSssSamplesPerChannel / 4; // Lobe exp(-r/d), probability 1/4
const int kSssSecondLobeCount = kSssSamplesPerChannel - kSssFirstLobeCount; // Lobe exp(-r/3d), probability 3/4
const int kSssSampleCount = 3 * kSssSamplesPerChannel;
const float kGoldenAngle = 2.39996323;
const float kDepthTolerance = 0.02; // Relative to the view depth, extra tolerance for depth rejection
const float kSssMaxRadiusPixels = 16.0; // Max on-screen mean radius of the side lobe

// Radial density of Burley's profile
float profileRadialPdf(float r, float d)
{
    return 0.25 * exp(-r / d) / d + 0.75 * exp(-r / (3.0 * d)) / (3.0 * d);
}

// Interleaved gradient noise (Jimenez 2014)
float interleavedGradientNoise(vec2 p)
{
    return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

vec3 blurSubsurface(vec2 uv, vec3 centerIrradiance, float centerDepth, vec3 sssDistance)
{
    vec3 d = max(sssDistance, vec3(1e-4));
    const float rotation = interleavedGradientNoise(gl_FragCoord.xy) * TWO_PI;
    const float jitter = interleavedGradientNoise(gl_FragCoord.xy + vec2(47.0, 17.0));
    const vec2 screenSize = vec2(textureSize(gSssIrradiance, 0));
    const float dMax = max(d.r, max(d.g, d.b));
    const float pixelsPerUnit = pc.uvPerUnit.x * screenSize.x / centerDepth;
    const float kernelScale = min(1.0, kSssMaxRadiusPixels / max(3.0 * dMax * pixelsPerUnit, 1e-4));

    vec3 weightedSum = vec3(0.0);
    vec3 weightSum = vec3(0.0);

    for (int i = 0; i < kSssSampleCount; ++i) {
        // Draw the radius from channel c's profile, stratified and jittered per pixel
        const int c = i / kSssSamplesPerChannel;
        const int j = i - c * kSssSamplesPerChannel;
        float r;
        if (j < kSssFirstLobeCount) {
            const float u = min((float(j) + jitter) / float(kSssFirstLobeCount), 0.995);
            r = -d[c] * log(1.0 - u);
        } else {
            const float u = min((float(j - kSssFirstLobeCount) + jitter) / float(kSssSecondLobeCount), 0.995);
            r = -3.0 * d[c] * log(1.0 - u);
        }

        const float theta = float(i) * kGoldenAngle + rotation;
        vec2 offsetUv = (kernelScale * r) * vec2(cos(theta), sin(theta)) * pc.uvPerUnit / centerDepth;
        const float offsetPixels = length(offsetUv * screenSize);
        if (offsetPixels > kSssMaxRadiusPixels) {
            offsetUv *= kSssMaxRadiusPixels / offsetPixels;
        }
        const vec2 sampleUv = uv + offsetUv;

        // Off-screen samples are invalid, so they fall back to the center irradiance instead of the screen edge
        const bool onScreen = all(greaterThanEqual(sampleUv, vec2(0.0))) && all(lessThanEqual(sampleUv, vec2(1.0)));
        const vec4 s = onScreen ? textureLod(gSssIrradiance, sampleUv, 0.0) : vec4(0.0);

        // One shared sample for all channels, pdf is the mix of the three channel profiles
        const vec3 channelPdf = vec3(profileRadialPdf(r, d.r), profileRadialPdf(r, d.g), profileRadialPdf(r, d.b));
        const float sharedPdf = (channelPdf.r + channelPdf.g + channelPdf.b) / 3.0;
        const vec3 w = channelPdf / max(sharedPdf, 1e-8);

        // Soft depth rejection, the rejected part falls back to the center irradiance
        const float tolerance = (kernelScale * r) + kDepthTolerance * centerDepth;
        const float depthWeight = s.a > 0.0 ? clamp(1.0 - abs(s.a - centerDepth) / tolerance, 0.0, 1.0) : 0.0;
        const vec3 irradiance = mix(centerIrradiance, s.rgb, depthWeight);

        weightedSum += w * irradiance;
        weightSum += w;
    }

    return weightedSum / max(weightSum, 1e-8);
}

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gSceneColor, 0));
    vec3 color = textureLod(gSceneColor, uv, 0.0).rgb;

    const vec4 irradiance = textureLod(gSssIrradiance, uv, 0.0);
    if (irradiance.a > 0.0) {
        const vec3 albedo = textureLod(gSssAlbedo, uv, 0.0).rgb;
        const vec3 sssDistance = max(textureLod(gSssScatter, uv, 0.0).rgb, vec3(0.0));

        // Blur the irradiance, then multiply by albedo (albedo is not blurred, so texture detail stays sharp)
        color += albedo * blurSubsurface(uv, irradiance.rgb, irradiance.a, sssDistance);
    }

    // Tone mapping after the subsurface contribution, so the blur happens in linear HDR
    vec3 ldrColor = color / (color + vec3(1.0));

    outColor = vec4(ldrColor, 1.0);
}
