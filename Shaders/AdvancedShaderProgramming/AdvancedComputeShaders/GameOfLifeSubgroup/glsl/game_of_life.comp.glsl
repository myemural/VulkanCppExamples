#version 450
#extension GL_KHR_shader_subgroup_ballot : require
#extension GL_KHR_shader_subgroup_vote : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 32, local_size_y = 32) in;

// Cell state: 0 = dead, 1 = alive. Ping-ponged across frames.
layout(binding = 0, r8ui) uniform readonly uimage2D prevState;
layout(binding = 1, r8ui) uniform writeonly uimage2D nextState;

// Visualization target sampled by the fragment shader.
layout(binding = 2, rgba8) uniform writeonly image2D destImage;

layout (push_constant) uniform PushConstants {
    float time;
    uint frameIndex; // 0 on the very first frame,triggers random seeding
} pc;

// GPU-side hash for seeding the initial random cell distribution
uint hash(uint x)
{
    x ^= x >> 16u;
    x *= 0x7feb352du;
    x ^= x >> 15u;
    x *= 0x846ca68bu;
    x ^= x >> 16u;
    return x;
}

bool isRandomAlive(ivec2 pixel)
{
    uint h = hash(uint(pixel.x) * 1973u + uint(pixel.y) * 9277u + 26699u);
    return (h % 100u) < 30u;
}

// Reads a single neighbor cell from prevState, treating out-of-bounds coordinates as dead (dead border)
uint loadCell(ivec2 pixel, ivec2 size)
{
    if (pixel.x < 0 || pixel.y < 0 || pixel.x >= size.x || pixel.y >= size.y) {
        return 0u;
    }
    return imageLoad(prevState, pixel).r;
}

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size  = imageSize(prevState);

    if (pixel.x >= size.x || pixel.y >= size.y) {
        return;
    }

    // First frame seeds a random initial state, and skips simulation
    if (pc.frameIndex == 0u) {
        bool isAlive = isRandomAlive(pixel);
        imageStore(nextState, pixel, uvec4(isAlive ? 1u : 0u));
        imageStore(destImage, pixel, isAlive ? vec4(1.0) : vec4(vec3(0.0), 1.0));
        return;
    }

    bool selfAlive = loadCell(pixel, size) != 0u;
    bool rowAboveAlive = loadCell(pixel + ivec2(0, -1), size) != 0u;
    bool rowBelowAlive = loadCell(pixel + ivec2(0,  1), size) != 0u;
    bool anyRelevantAlive = selfAlive || rowAboveAlive || rowBelowAlive;

    // No live cell exists in this subgroup's 3-row neighborhood
    if (!subgroupAny(anyRelevantAlive)) {
        imageStore(nextState, pixel, uvec4(0u));
        imageStore(destImage, pixel, vec4(vec3(0.0), 1.0));
        return;
    }

    // Get horizontal neighbors from the lane mask
    uvec4 ballot = subgroupBallot(selfAlive);
    uint lane = gl_SubgroupInvocationID;
    bool leftAliveFromBallot = (lane > 0u) && subgroupBallotBitExtract(ballot, lane - 1u);
    bool rightAliveFromBallot = (lane < gl_SubgroupSize - 1u) && subgroupBallotBitExtract(ballot, lane + 1u);

    bool leftAlive, rightAlive;
    if (lane > 0u) {
        // Left neighbor is another lane in this same subgroup/row, so use ballot
        leftAlive = leftAliveFromBallot;
    } else {
        // Lane 0 has no "lane - 1" inside the subgroup, so fall back to a direct load
        leftAlive = loadCell(pixel + ivec2(-1, 0), size) != 0u;
    }

    if (lane < gl_SubgroupSize - 1u) {
        // Right neighbor is another lane in this same subgroup/row, so use ballot
        rightAlive = rightAliveFromBallot;
    } else {
        // Last lane in the subgroup, so fall back to a direct load
        rightAlive = loadCell(pixel + ivec2(1, 0), size) != 0u;
    }

    // Diagonal neighbors require direct loads
    bool topLeftAlive = loadCell(pixel + ivec2(-1, -1), size) != 0u;
    bool topRightAlive = loadCell(pixel + ivec2( 1, -1), size) != 0u;
    bool bottomLeftAlive = loadCell(pixel + ivec2(-1,  1), size) != 0u;
    bool bottomRightAlive = loadCell(pixel + ivec2( 1,  1), size) != 0u;

    uint liveNeighbors = uint(leftAlive) + uint(rightAlive) + uint(rowAboveAlive) + uint(rowBelowAlive) +
                         uint(topLeftAlive) + uint(topRightAlive) + uint(bottomLeftAlive) + uint(bottomRightAlive);

    // Classic Conway's Game of Life rules
    bool nextAlive;
    if (selfAlive) {
        nextAlive = (liveNeighbors == 2u) || (liveNeighbors == 3u);
    } else {
        nextAlive = (liveNeighbors == 3u);
    }

    // Low-rate noise keeps the bounded simulation from becoming static
    uint noiseHash = hash(uint(pixel.x) * 92821u + uint(pixel.y) * 68917u + pc.frameIndex * 2654435761u);
    if ((noiseHash % 100000u) < 5u) {
        nextAlive = true;
    }

    imageStore(nextState, pixel, uvec4(nextAlive ? 1u : 0u));

    // Highlights subgroups whose lanes are all alive
    bool subgroupUnanimousAlive = subgroupAll(nextAlive);
    vec3 color = nextAlive ? (subgroupUnanimousAlive ? vec3(0.2, 1.0, 0.3) : vec3(1.0)) : vec3(0.0);

    imageStore(destImage, pixel, vec4(color, 1.0));
}
