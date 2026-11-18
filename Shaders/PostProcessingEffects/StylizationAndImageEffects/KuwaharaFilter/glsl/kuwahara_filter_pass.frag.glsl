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

layout(push_constant) uniform KuwaharaPushConstants {
    float exposure;    // Manual exposure in EV
    uint kernelRadius; // Radius of the filter kernel in pixels
    uint filterMode;   // 0: Disabled, 1: Basic, 2: Generalized, 3: Anisotropic
} pc;

const uint kModeBasic = 1U;
const uint kModeGeneralized = 2U;
const uint kModeAnisotropic = 3U;

const int kSectorCount = 8;

// Sharpness of the variance weighting (larger: closer to picking only the sector with the lowest variance)
const float kSharpness = 8.0;

// Scale of the variances before the weighting (the colors are in [0, 1])
const float kVarianceScale = 1000.0;

// Angle of the zero crossing of the sector weights from the sector axis (22.5 degrees is the half sector)
const float kZeroCrossing = 0.58;

// Controls how much the ellipse is stretched by the anisotrophy
const float kAnisotrophyAlpha = 1.0;

ivec2 gImageSize;

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

// Display color of a pixel (clamped to the image borders)
vec3 fetchColor(ivec2 position)
{
    vec3 hdrColor = texelFetch(gLightPassOutput, clamp(position, ivec2(0), gImageSize - 1), 0).rgb;
    return tonemapAces(hdrColor * exp2(pc.exposure));
}

vec3 basicKuwahara(ivec2 center, int radius)
{
    // Sub-regions: 0: top-left, 1: top-right, 2: bottom-left, 3: bottom-right (they share the center row/column)
    vec3 mean[4] = vec3[](vec3(0.0), vec3(0.0), vec3(0.0), vec3(0.0));
    vec3 squareMean[4] = vec3[](vec3(0.0), vec3(0.0), vec3(0.0), vec3(0.0));

    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            vec3 color = fetchColor(center + ivec2(x, y));
            vec3 colorSquared = color * color;
            if (x <= 0 && y <= 0) { mean[0] += color; squareMean[0] += colorSquared; }
            if (x >= 0 && y <= 0) { mean[1] += color; squareMean[1] += colorSquared; }
            if (x <= 0 && y >= 0) { mean[2] += color; squareMean[2] += colorSquared; }
            if (x >= 0 && y >= 0) { mean[3] += color; squareMean[3] += colorSquared; }
        }
    }

    float sampleCount = float((radius + 1) * (radius + 1));
    vec3 result = vec3(0.0);
    float minVariance = 1e30;
    for (int i = 0; i < 4; ++i) {
        vec3 m = mean[i] / sampleCount;
        vec3 variance = squareMean[i] / sampleCount - m * m;
        float totalVariance = variance.r + variance.g + variance.b;
        if (totalVariance < minVariance) {
            minVariance = totalVariance;
            result = m;
        }
    }

    return result;
}

// Structure tensor (E = fx.fx, F = fx.fy, G = fy.fy) of the color image, from the central difference gradients in a
// 5x5 neighborhood with Gaussian weights (smoothing of the tensor gives a stable orientation)
vec3 structureTensor(ivec2 center)
{
    // Colors of the 7x7 neighborhood are cached, because each gradient needs the neighbors
    vec3 colors[49];
    for (int y = -3; y <= 3; ++y) {
        for (int x = -3; x <= 3; ++x) {
            colors[(y + 3) * 7 + (x + 3)] = fetchColor(center + ivec2(x, y));
        }
    }

    vec3 tensor = vec3(0.0);
    float weightSum = 0.0;
    for (int y = -2; y <= 2; ++y) {
        for (int x = -2; x <= 2; ++x) {
            int index = (y + 3) * 7 + (x + 3);
            vec3 fx = 0.5 * (colors[index + 1] - colors[index - 1]);
            vec3 fy = 0.5 * (colors[index + 7] - colors[index - 7]);
            float weight = exp(-float(x * x + y * y) / (2.0 * 1.5 * 1.5));
            tensor += weight * vec3(dot(fx, fx), dot(fx, fy), dot(fy, fy));
            weightSum += weight;
        }
    }

    return tensor / weightSum;
}

// Polynomial weights of the four sectors along the axes (+y, -x, -y, +x) for a position in the unit disk
vec4 sectorWeights(vec2 v, float zeta, float eta)
{
    float vxx = zeta - eta * v.x * v.x;
    float vyy = zeta - eta * v.y * v.y;
    vec4 z = max(vec4(v.y + vxx, -v.x + vyy, -v.y + vxx, v.x + vyy), 0.0);

    return z * z;
}

vec3 generalizedKuwahara(ivec2 center, int radius, bool isAnisotropic)
{
    // Ellipse of the kernel: Circle for the generalized filter
    // For the anisotropic filter, the major axis follows the edge direction and the ellipse is stretched with the anisotrophy
    float a = float(radius);
    float b = float(radius);
    float phi = 0.0;
    if (isAnisotropic) {
        vec3 tensor = structureTensor(center);
        float e = tensor.x;
        float f = tensor.y;
        float g = tensor.z;
        float root = sqrt((e - g) * (e - g) + 4.0 * f * f);
        float lambda1 = 0.5 * (e + g + root);
        float lambda2 = 0.5 * (e + g - root);

        vec2 edgeDirection = vec2(lambda1 - e, -f);
        edgeDirection = (length(edgeDirection) > 1e-8) ? normalize(edgeDirection) : vec2(0.0, 1.0);
        phi = atan(edgeDirection.y, edgeDirection.x);

        float anisotrophy = (lambda1 + lambda2 > 1e-8) ? (lambda1 - lambda2) / (lambda1 + lambda2) : 0.0;
        a = float(radius) * clamp((kAnisotrophyAlpha + anisotrophy) / kAnisotrophyAlpha, 0.1, 2.0);
        b = float(radius) * clamp(kAnisotrophyAlpha / (kAnisotrophyAlpha + anisotrophy), 0.1, 2.0);
    }

    // Pixel offset to unit disk: Rotate the ellipse axes, then scale with the semi-axes
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);
    mat2 rotation = mat2(cosPhi, -sinPhi, sinPhi, cosPhi); // Rotation by -phi (columns)
    mat2 toUnitDisk = mat2(1.0 / a, 0.0, 0.0, 1.0 / b) * rotation;

    // Bounding box of the ellipse
    int maxX = int(ceil(sqrt(a * a * cosPhi * cosPhi + b * b * sinPhi * sinPhi)));
    int maxY = int(ceil(sqrt(a * a * sinPhi * sinPhi + b * b * cosPhi * cosPhi)));

    // Sector weights: Parabolas which cross zero on the unit circle at kZeroCrossing from the sector axis
    float zeta = 2.0 / float(radius);
    float sinZeroCrossing = sin(kZeroCrossing);
    float eta = (zeta + cos(kZeroCrossing)) / (sinZeroCrossing * sinZeroCrossing);

    vec4 mean[kSectorCount];
    vec3 squareMean[kSectorCount];
    for (int k = 0; k < kSectorCount; ++k) {
        mean[k] = vec4(0.0);
        squareMean[k] = vec3(0.0);
    }

    const float halfSqrt2 = 0.70710678;
    for (int y = -maxY; y <= maxY; ++y) {
        for (int x = -maxX; x <= maxX; ++x) {
            vec2 v = toUnitDisk * vec2(x, y);
            float distanceSquared = dot(v, v);
            if (distanceSquared > 1.0) {
                continue;
            }

            // Weights of the axis aligned sectors (0, 2, 4, 6) and the diagonal sectors (1, 3, 5, 7, rotated by 45)
            vec4 axisWeights = sectorWeights(v, zeta, eta);
            vec4 diagonalWeights = sectorWeights(halfSqrt2 * vec2(v.x - v.y, v.x + v.y), zeta, eta);
            float weightSum = dot(axisWeights, vec4(1.0)) + dot(diagonalWeights, vec4(1.0));

            // Gaussian falloff, normalized so the sector weights of a pixel sum to the Gaussian
            float gaussian = exp(-2.0 * distanceSquared) / max(weightSum, 1e-6);

            vec3 color = fetchColor(center + ivec2(x, y));
            vec3 colorSquared = color * color;
            for (int k = 0; k < 4; ++k) {
                float axisWeight = axisWeights[k] * gaussian;
                float diagonalWeight = diagonalWeights[k] * gaussian;
                mean[2 * k] += vec4(color * axisWeight, axisWeight);
                squareMean[2 * k] += colorSquared * axisWeight;
                mean[2 * k + 1] += vec4(color * diagonalWeight, diagonalWeight);
                squareMean[2 * k + 1] += colorSquared * diagonalWeight;
            }
        }
    }

    // Blend the sector means: Sectors with lower variance (no edge inside) get much larget weights
    vec3 result = vec3(0.0);
    float resultWeight = 0.0;
    for (int k = 0; k < kSectorCount; ++k) {
        if (mean[k].w <= 0.0) {
            continue;
        }

        vec3 m = mean[k].rgb / mean[k].w;
        vec3 variance = abs(squareMean[k] / mean[k].w - m * m);
        float totalVariance = variance.r + variance.g + variance.b;
        float weight = 1.0 / (1.0 + pow(kVarianceScale * totalVariance, 0.5 * kSharpness));
        result += m * weight;
        resultWeight += weight;
    }

    return result / max(resultWeight, 1e-8);
}

void main()
{
    gImageSize = textureSize(gLightPassOutput, 0);
    ivec2 center = ivec2(gl_FragCoord.xy);
    int radius = int(max(pc.kernelRadius, 1U));

    vec3 color;
    if (pc.filterMode == kModeBasic) {
        color = basicKuwahara(center, radius);
    } else if (pc.filterMode == kModeGeneralized || pc.filterMode == kModeAnisotropic) {
        color = generalizedKuwahara(center, radius, pc.filterMode == kModeAnisotropic);
    } else {
        color = fetchColor(center);
    }

    outColor = vec4(color, 1.0);
}
