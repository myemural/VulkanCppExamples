#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

layout(std430, binding = 0) buffer DistributionIn { float f[]; };
layout(std430, binding = 2) buffer DyeIn { float dye[]; };
layout(std430, binding = 4) buffer ObstacleMask { uint obstacle[]; };

layout (push_constant) uniform PushConstants {
    uint gridWidth;
    uint gridHeight;
    float relaxationTime;
    float inletVelocity;
    uint stepIndex;    // Global LBM step counter, 0 triggers initialization
    uint displayMode;
    vec2 mousePos;      // In pixels = Lattice cells
    float brushRadius;
    uint mouseMode;
} pc;

const ivec2 C[9] = ivec2[9](ivec2(0, 0), ivec2(1, 0), ivec2(0, 1), ivec2(-1, 0), ivec2(0, -1), ivec2(1, 1),
        ivec2(-1, 1), ivec2(-1, -1), ivec2(1, -1));
const float W[9] = float[9](4.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0,
        1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0);

uint cellIndex(ivec2 p)
{
    return uint(p.y) * pc.gridWidth + uint(p.x);
}

uint fIndex(uint q, uint cell)
{
    return q * (pc.gridWidth * pc.gridHeight) + cell;
}

float equilibrium(uint q, float rho, vec2 u)
{
    float cu = dot(vec2(C[q]), u);
    return W[q] * rho * (1.0 + 3.0 * cu + 4.5 * cu * cu - 1.5 * dot(u, u));
}

void main()
{
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    if (pos.x >= int(pc.gridWidth) || pos.y >= int(pc.gridHeight)) {
        return;
    }

    if (distance(vec2(pos) + 0.5, pc.mousePos) > pc.brushRadius) {
        return;
    }

    uint cell = cellIndex(pos);
    bool isAdding = pc.mouseMode == 1U;

    obstacle[cell] = isAdding ? 1U : 0U;
    dye[cell] = 0.0;

    // Solid cells rest, freed cells restart from the inlet flow
    vec2 u = isAdding ? vec2(0.0) : vec2(pc.inletVelocity, 0.0);
    for (uint q = 0U; q < 9U; ++q) {
        f[fIndex(q, cell)] = equilibrium(q, 1.0, u);
    }
}
