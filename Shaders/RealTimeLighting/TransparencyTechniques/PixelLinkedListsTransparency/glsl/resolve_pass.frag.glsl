#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define NULL_NODE 0xFFFFFFFFu
#define MAX_FRAGMENTS 8U

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUv;

struct OitNode
{
    uint colorPacked;
    float depth;
    uint next;
};

layout(std430, set = 0, binding = 0) readonly buffer OitLinkedListBuffer
{
    OitNode nodes[];
};

layout(std430, set=0, binding = 1) readonly buffer OitHeadPointerBuffer
{
    uint heads[];
};

layout(push_constant) uniform ResolvePassPushConstants {
    uint screenWidth;
} pc;

void SortFragments(inout vec4 colors[MAX_FRAGMENTS], inout float depths[MAX_FRAGMENTS], int count)
{
    for (int i = 1; i < count; ++i) {
        vec4  keyColor = colors[i];
        float keyDepth = depths[i];
        int j = i - 1;
        while (j >= 0 && depths[j] < keyDepth) {
            colors[j + 1] = colors[j];
            depths[j + 1] = depths[j];
            --j;
        }
        colors[j + 1] = keyColor;
        depths[j + 1] = keyDepth;
    }
}

void main()
{
    // Calculate pixel index
    ivec2 coords = ivec2(gl_FragCoord.xy);
    uint pixelIdx = uint(coords.y) * pc.screenWidth + uint(coords.x);

    uint head = heads[pixelIdx];
    if (head == NULL_NODE) {
        discard;
    }

    vec4 colors[MAX_FRAGMENTS];
    float depths[MAX_FRAGMENTS];
    int count = 0;

    uint nodeIdx = head;
    while (nodeIdx != NULL_NODE && count < MAX_FRAGMENTS) {
        colors[count] = unpackUnorm4x8(nodes[nodeIdx].colorPacked);
        depths[count] = nodes[nodeIdx].depth;
        nodeIdx = nodes[nodeIdx].next;
        ++count;
    }

    SortFragments(colors, depths, count);

    vec4 result = vec4(0.0);
    for (int i = 0; i < count; ++i) {
        vec4 src = colors[i];
        result.rgb = src.rgb * src.a + result.rgb * (1.0 - src.a);
        result.a   = src.a   + result.a * (1.0 - src.a);
    }

    outColor = result;
}