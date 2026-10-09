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

layout(push_constant) uniform LensDistortionPushConstants {
    float exposure;           // Manual exposure in EV
    float distortionStrength; // Amount of the distortion, range [0, 1]
    float zoom;               // Extra zoom applied to the image (> 1.0 zooms in)
    // Distortion Mode
    // 0: Barrel
    // 1: Pincushion
    // 2: Fisheye
    uint distortionMode;
    uint isAutoFit;            // 0: Disabled, 1: Scales the barrel distorted image so that no black border is visible
    uint isDistortionEnabled;  // 0: Lens distortion disabled (only tone mapping), 1: Lens distortion enabled
} pc;

const float kHalfPi = 1.57079632679;

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

// Brown-Conrady radial distortion polynomial: f(r) = 1 + k1 * r^2 + k2 * r^4
// It is used in the inverse direction (from output pixel to source pixel)
//  k > 0: Samples are pushed outwards, image is compressed towards the edges (Barrel)
//  k < 0: Samples are pulled inwards, image is stretched towards the edges (Pincushion)
// Reference: https://en.wikipedia.org/wiki/Distortion_(optics)
float radialDistortionFactor(float r2, float k1, float k2)
{
    return 1.0 + k1 * r2 + k2 * r2 * r2;
}

// Barrel distortion, returns the resource position for the given centered (aspect corrected) position
vec2 distortedBarrel(vec2 p, float maxR2)
{
    float k1 = 0.5 * pc.distortionStrength;
    float k2 = 0.25 * pc.distortionStrength;

    vec2 distorted = p * radialDistortionFactor(dot(p, p), k1, k2);

    // Scale the image, so that the screen corners map exactly to the source corners (no black border)
    if (pc.isAutoFit != 0U) {
        distorted /= radialDistortionFactor(maxR2, k1, k2);
    }

    return distorted;
}

// Pincushion distortion, returns the source position for the given cenrtered (aspect corrected) position
vec2 distortPincushion(vec2 p, float maxR2)
{
    // The mapping "r * (1 + k * r^2)" must stay monotonic up to the corners "(1 + 3 * k * r^2 > 0)"
    // Otherwise, the image folds onto itself
    float k1 = -pc.distortionStrength * 0.95 / (3.0 * maxR2);
    return p * radialDistortionFactor(dot(p, p), k1, 0.0);
}

// Fisheye distortion (equidistant projection), returns the soruce position for the given centered position
// Output is treated as an equidistant fisheye image "(r =  * theta)", source is a rectilinear image "(r = f * tan(theta))"
// The radius is normalized by the corner radius, so the result is a full-frame (diagonal) fisheye
// Reference: https://en.wikipedia.org/wiki/Fisheye_lens#Mapping_function
vec2 distortFisheye(vec2 p, float maxR2)
{
    float maxR = sqrt(maxR2);
    float r = length(p);
    if (r < 1e-5) {
        return p;
    }

    // Half of the diagonal FOV, strength 1.0 gives ~172 degrees diagonal FOV
    float maxTheta = max(pc.distortionStrength * (kHalfPi - 0.07), 1e-3);
    float theta = (r / maxR) * maxTheta;

    // Rays at 90 degrees or more can't be projected onto a rectilinear image (zoom < 1.0 reveals this area)
    // So, they area moved far outside of the image to be shown as black (circular fisheye look)
    if (theta >= kHalfPi - 1e-3) {
        return p * 1e3;
    }

    float sourceR = (tan(theta) / tan(maxTheta)) * maxR;

    return p * (sourceR / r);
}

void main()
{
    vec2 resolution = vec2(textureSize(gLightPassOutput, 0));
    vec2 uv = gl_FragCoord.xy / resolution;

    if (pc.isDistortionEnabled != 0U) {
        // Move to the centered space and correct the aspect ratio, so the distortion is radially symmetric
        float aspectRatio = resolution.x / resolution.y;
        vec2 aspect = vec2(aspectRatio, 1.0);
        vec2 p = (uv - 0.5) * aspect;
        float maxR2 = dot(0.5 * aspect, 0.5 * aspect); // Squared distance from center to the corner

        p /= max(pc.zoom, 0.01);

        if (pc.distortionMode == 0U) {
            p = distortedBarrel(p, maxR2);
        } else if (pc.distortionMode == 1U) {
            p = distortPincushion(p, maxR2);
        } else {
            p = distortFisheye(p, maxR2);
        }

        uv = p / aspect + 0.5;

        // Samples outside of the source image are shown as black (like a lens border)
        if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) {
            outColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
    }

    // Sampling the HDR output of the light pass
    vec3 hdrColor = texture(gLightPassOutput, uv).xyz;

    // Manual exposure control
    vec3 exposedColor = hdrColor * exp2(pc.exposure);

    // Apply ACES tonemapping (HDR to LDR)
    outColor = vec4(tonemapAces(exposedColor), 1.0);
}
