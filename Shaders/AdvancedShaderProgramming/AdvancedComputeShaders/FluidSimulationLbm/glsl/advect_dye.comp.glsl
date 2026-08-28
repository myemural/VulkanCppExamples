#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

layout(std430, binding = 1) buffer DistributionOut { float f[]; };
layout(std430, binding = 2) buffer DyeIn { float dyeIn[]; };
layout(std430, binding = 3) buffer DyeOut { float dyeOut[]; };
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
const int kStripeHeight = 16;

uint cellIndex(ivec2 p)
{
    return uint(p.y) * pc.gridWidth + uint(p.x);
}

uint fIndex(uint q, uint cell)
{
    return q * (pc.gridWidth * pc.gridHeight) + cell;
}

vec2 velocityAt(uint cell)
{
    vec2 momentum = vec2(0.0);
    float rho = 0.0;
    for (uint q = 0U; q < 9U; ++q) {
        float fq = f[fIndex(q, cell)];
        rho += fq;
        momentum += fq * vec2(C[q]);
    }
    return momentum / max(rho, 1e-5);
}

// Bilinear fetch of the dye field with clamped coordinates
float sampleDye(vec2 p)
{
    vec2 maxPos = vec2(float(pc.gridWidth) - 1.0, float(pc.gridHeight) - 1.0);
    p = clamp(p, vec2(0.0), maxPos);

    ivec2 base = ivec2(floor(p));
    ivec2 next = min(base + 1, ivec2(maxPos));
    vec2 t = p - vec2(base);

    float d00 = dyeIn[cellIndex(ivec2(base.x, base.y))];
    float d10 = dyeIn[cellIndex(ivec2(next.x, base.y))];
    float d01 = dyeIn[cellIndex(ivec2(base.x, next.y))];
    float d11 = dyeIn[cellIndex(ivec2(next.x, next.y))];

    return mix(mix(d00, d10, t.x), mix(d01, d11, t.x), t.y);
}

void main()
{
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    if (pos.x >= int(pc.gridWidth) || pos.y >= int(pc.gridHeight)) {
        return;
    }

    uint cell = cellIndex(pos);

    if (pc.stepIndex == 0U || obstacle[cell] != 0U) {
        dyeOut[cell] = 0.0;
        return;
    }

    // Inlet, alternating horizontal stripes
    if (pos.x < 2) {
        dyeOut[cell] = ((pos.y / kStripeHeight) % 2 == 0) ? 1.0 : 0.0;
        return;
    }

    // Trace back one lattice time step and resample
    vec2 source = vec2(pos) - velocityAt(cell);
    dyeOut[cell] = sampleDye(source) * 0.999;
}
