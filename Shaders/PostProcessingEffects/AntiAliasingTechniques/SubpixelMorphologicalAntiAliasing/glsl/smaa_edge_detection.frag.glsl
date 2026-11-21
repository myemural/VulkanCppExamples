#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec2 outEdges; // r = Left edge, g = Top edge

layout(set = 0, binding = 0) uniform sampler2D gColor; // Tone mapped color in display (sRGB) space

layout(push_constant) uniform SmaaPushConstants {
    float threshold;               // Luma difference threshold of the edges
    uint maxSearchSteps;           // Search steps of the horizontal/vertical lines
    uint maxSearchStepsDiag;       // Search steps of the diagonal lines (0: Diagonal detection disabled)
    uint isCornerDetectionEnabled; // 0: Sharp corners are blended, 1: Sharp corners are preserved
    uint isSmaaEnabled;            // 0: SMAA disabled, 1: SMAA enabled
    uint debugView;                // 0: Final image, 1: Edges, 2: Blending weights
} pc;

const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);
const float kLocalContrastAdaptationFactor = 2.0;

ivec2 gImageSize;

float lumaAt(ivec2 position)
{
    vec3 color = texelFetch(gColor, clamp(position, ivec2(0), gImageSize - 1), 0).rgb;
    return dot(color, kLumaRec709);
}

void main()
{
    gImageSize = textureSize(gColor, 0);
    ivec2 position = ivec2(gl_FragCoord.xy);

    // Differences to the left and top neighbors
    float luma = lumaAt(position);
    float lumaLeft = lumaAt(position + ivec2(-1, 0));
    float lumaTop = lumaAt(position + ivec2(0, -1));

    vec2 delta = abs(luma - vec2(lumaLeft, lumaTop));
    vec2 edges = step(vec2(pc.threshold), delta);

    if (dot(edges, vec2(1.0)) == 0.0) {
        discard;
    }

    // Largest difference in the neighborhood: Right, bottom, left, left-left and top-top neighbors
    float lumaRight = lumaAt(position + ivec2(1, 0));
    float lumaBottom = lumaAt(position + ivec2(0, 1));
    float lumaLeftLeft = lumaAt(position + ivec2(-2, 0));
    float lumaTopTop = lumaAt(position + ivec2(0, -2));

    vec2 maxDelta = max(delta, abs(luma - vec2(lumaRight, lumaBottom)));
    maxDelta = max(maxDelta, abs(vec2(lumaLeft, lumaTop) - vec2(lumaLeftLeft, lumaTopTop)));
    float finalDelta = max(maxDelta.x, maxDelta.y);

    // Local contrast adaptation
    edges *= step(vec2(finalDelta), kLocalContrastAdaptationFactor * delta);

    outEdges = edges;
}
