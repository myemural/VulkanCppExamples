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
layout(set = 0, binding = 2) uniform sampler2D gGeometricNormal; // View-space vertex normal (without normal maps)

layout(push_constant) uniform EdgeDetectionPushConstants {
    float exposure;          // Manual exposure in EV
    float outlineThickness;  // Distance of the sobel samples in pixels (thicker outlines with larget values)
    uint edgeSource;         // 0: Depth + normal, 1: Depth, 2: Normal, 3: Color (Luminance)
    uint isOutlineEnabled;   // 0: Outlines disabled (only tone mapping), 1: Outlines enabled
    uint debugView;          // 0: Final image, 1: Edges only
} pc;

const uint EDGE_SOURCE_DEPTH_NORMAL = 0U;
const uint EDGE_SOURCE_DEPTH = 1U;
const uint EDGE_SOURCE_NORMAL = 2U;
const uint EDGE_SOURCE_COLOR = 3U;

const uint DEBUG_VIEW_EDGES = 1U;

const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);

// Edge thresholds (start of the smooth transition, the edge is full at two times of the threshold)
const float kDepthThreshold = 0.05; // Relatvie depth difference
const float kGrazingFactor = 6.0;    // Increases the depth threshold on the surfaces seen at grazing angles
const float kNormalThreshold = 0.5; // Normal difference
const float kColorThreshold = 0.25; // Luminance difference

const vec3 kOutlineColor = vec3(1.0, 0.5, 0.3);

// Pixels without geometry (cleared G-Buffer) are treated as very far away
const float kBackgroundDepth = 10000.0;

// Sobel kernels (3x3 neighborhood, row by row from top-left)
const float kSobelX[9] = float[](-1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0);
const float kSobelY[9] = float[](-1.0, -2.0, -1.0, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0);

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

vec2 clampUv(vec2 uv, vec2 resolution)
{
    // Shared sampler uses REPEAT address mode, so the coordinates are clamped to the image (half texel inside)
    return clamp(uv, 0.5 / resolution, 1.0 - 0.5 / resolution);
}

vec4 readPosition(vec2 uv)
{
    vec4 position = texture(gPosition, uv);
    return position.w > 0.0 ? position : vec4(0.0, 0.0, -kBackgroundDepth, 0.0);
}

vec3 readNormal(vec2 uv)
{
    vec4 normal = texture(gGeometricNormal, uv);
    return normal.w > 0.0 ? normalize(normal.xyz) : vec3(0.0);
}

float luminance(vec3 color)
{
    return dot(color, kLumaRec709);
}

vec3 toDisplayColor(vec3 hdrColor)
{
    return tonemapAces(hdrColor * exp2(pc.exposure));
}

float detectEdges(vec2 uv, vec2 resolution)
{
    vec2 texelSize = 1.0 / resolution;
    vec2 offsetScale = texelSize * pc.outlineThickness;

    bool useDepth = pc.edgeSource == EDGE_SOURCE_DEPTH_NORMAL || pc.edgeSource == EDGE_SOURCE_DEPTH;
    bool useNormal = pc.edgeSource == EDGE_SOURCE_DEPTH_NORMAL || pc.edgeSource == EDGE_SOURCE_NORMAL;
    bool useColor = pc.edgeSource == EDGE_SOURCE_COLOR;

    float depthGx = 0.0;
    float depthGy = 0.0;
    vec3 normalGx = vec3(0.0);
    vec3 normalGy = vec3(0.0);
    float colorGx = 0.0;
    float colorGy = 0.0;

    for (int i = 0; i < 9; ++i) {
        vec2 sampleUv = clampUv(uv + vec2(float(i % 3 - 1), float(i / 3 - 1)) * offsetScale, resolution);
        if (useDepth) {
            float depth = -readPosition(sampleUv).z;
            depthGx += kSobelX[i] * depth;
            depthGy += kSobelY[i] * depth;
        }
        if (useNormal) {
            vec3 normal = readNormal(sampleUv);
            normalGx += kSobelX[i] * normal;
            normalGy += kSobelY[i] * normal;
        }
        if (useColor) {
            float luma = luminance(toDisplayColor(texture(gLightPassOutput, sampleUv).rgb));
            colorGx += kSobelX[i] * luma;
            colorGy += kSobelY[i] * luma;
        }
    }

    float edge = 0.0;

    if (useDepth) {
        // Depth difference relative to the center depth, so the edges are found at every distance
        vec4 centerPosition = readPosition(uv);
        float centerDepth = max(-centerPosition.z, 1e-3);
        float depthEdge = length(vec2(depthGx, depthGy)) / centerDepth;

        // A surface seen at a grazing angle (floor far away) has a large depth gradient without any edge
        // So the threshold is increased with the angle between the surface and the view directon
        vec3 normal = readNormal(uv);
        float normalDotView = abs(dot(normal, normalize(-centerPosition.xyz)));
        float threshold = kDepthThreshold * (1.0 + kGrazingFactor * (1.0 - normalDotView));
        edge = max(edge, smoothstep(threshold, 2.0 * threshold, depthEdge));
    }

    if (useNormal) {
        float normalEdge = sqrt(dot(normalGx, normalGx) + dot(normalGy, normalGy));
        edge = max(edge, smoothstep(kNormalThreshold, 2.0 * kNormalThreshold, normalEdge));
    }

    if (useColor) {
        float colorEdge = length(vec2(colorGx, colorGy));
        edge = max(edge, smoothstep(kColorThreshold, 2.0 * kColorThreshold, colorEdge));
    }

    return edge;
}

void main()
{
    vec2 resolution = vec2(textureSize(gLightPassOutput, 0));
    vec2 uv = gl_FragCoord.xy / resolution;

    // Outlines are drawn on the display color (after exposure and tone mapping)
    vec3 color = toDisplayColor(texture(gLightPassOutput, uv).rgb);

    if (pc.isOutlineEnabled != 0U || pc.debugView == DEBUG_VIEW_EDGES) {
        float edge = detectEdges(uv, resolution);
        color = (pc.debugView == DEBUG_VIEW_EDGES) ? vec3(1.0 - edge) : mix(color, kOutlineColor, edge);
    }

    outColor = vec4(color, 1.0);
}
