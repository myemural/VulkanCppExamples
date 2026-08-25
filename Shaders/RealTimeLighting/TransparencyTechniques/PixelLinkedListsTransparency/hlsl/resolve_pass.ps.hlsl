// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define NULL_NODE 0xFFFFFFFFu
#define MAX_FRAGMENTS 8U

struct PSInput
{
    float4 position : SV_Position;
    [[vk::location(1)]] float2 fragUv : TEXCOORD0;
};

struct OitNode
{
    uint colorPacked;
    float depth;
    uint next;
};
[[vk::binding(0, 0)]] RWStructuredBuffer<OitNode> nodes;

[[vk::binding(1, 0)]] RWStructuredBuffer<uint> heads;

struct MeshPushConstants
{
    uint screenWidth;
};
[[vk::push_constant]] MeshPushConstants pc;

float4 unpackUnorm4x8(uint packed)
{
    uint4 value = uint4(
        packed & 0xFF,
        (packed >> 8) & 0xFF,
        (packed >> 16) & 0xFF,
        (packed >> 24) & 0xFF
    );

    return float4(value) / 255.0;
}

void sortFragments(inout float4 colors[MAX_FRAGMENTS], inout float depths[MAX_FRAGMENTS], int count)
{
    for (int i = 1; i < count; ++i) {
        float4  keyColor = colors[i];
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

float4 main(PSInput input) : SV_Target
{
    // Calculate pixel index
    int2 coords = int2(input.position.xy);
    uint pixelIdx = uint(coords.y) * pc.screenWidth + uint(coords.x);

    uint head = heads[pixelIdx];
    if (head == NULL_NODE) {
        discard;
    }

    float4 colors[MAX_FRAGMENTS];
    float depths[MAX_FRAGMENTS];
    int count = 0;

    uint nodeIdx = head;
    while (nodeIdx != NULL_NODE && count < MAX_FRAGMENTS) {
        colors[count] = unpackUnorm4x8(nodes[nodeIdx].colorPacked);
        depths[count] = nodes[nodeIdx].depth;
        nodeIdx = nodes[nodeIdx].next;
        ++count;
    }

    sortFragments(colors, depths, count);

    float4 result = float4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < count; ++i) {
        float4 src = colors[i];
        result.rgb = src.rgb * src.a + result.rgb * (1.0 - src.a);
        result.a   = src.a   + result.a * (1.0 - src.a);
    }

    return result;
}
