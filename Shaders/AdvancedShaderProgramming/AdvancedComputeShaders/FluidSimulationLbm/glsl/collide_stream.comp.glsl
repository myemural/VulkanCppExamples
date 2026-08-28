#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

layout(std430, binding = 0) buffer DistributionIn { float fIn[]; };
layout(std430, binding = 1) buffer DistributionOut { float fOut[]; };
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

// D2Q9 directions, weights and reflection tables
const ivec2 C[9] = ivec2[9](ivec2(0, 0), ivec2(1, 0), ivec2(0, 1), ivec2(-1, 0), ivec2(0, -1), ivec2(1, 1),
                            ivec2(-1, 1), ivec2(-1, -1), ivec2(1, -1));
const float W[9] = float[9](4.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0,
                            1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0, 1.0 / 36.0);
const int kOpposite[9] = int[9](0, 3, 4, 1, 2, 7, 8, 5, 6); // 180 degree reflection
const int kMirrorY[9] = int[9](0, 1, 4, 3, 2, 8, 7, 6, 5); // Y-mirrored direction
const float kCylinderDiameter = 48.0; // Obstacle cylinder

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

void writeEquilibrium(uint cell, float rho, vec2 u)
{
    for (uint q = 0U; q < 9U; ++q) {
        fOut[fIndex(q, cell)] = equilibrium(q, rho, u);
    }
}

void main()
{
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    if (pos.x >= int(pc.gridWidth) || pos.y >= int(pc.gridHeight)) {
        return;
    }

    uint cell = cellIndex(pos);
    vec2 cylinderCenter = vec2(float(pc.gridWidth) * 0.25, float(pc.gridHeight) * 0.5 + 0.5);

    // Initialization, uniform flow plus the cylinder mask
    if (pc.stepIndex == 0) {
        bool isSolid = distance(vec2(pos), cylinderCenter) < kCylinderDiameter * 0.5;
        obstacle[cell] = isSolid ? 1U : 0U;
        writeEquilibrium(cell, 1.0, isSolid ? vec2(0.0) : vec2(pc.inletVelocity, 0.0));
        return;
    }

    // Solid cells keep a resting equilibrium so neighboring bounce-backs stay well-defined
    if (obstacle[cell] != 0U) {
        writeEquilibrium(cell, 1.0, vec2(0.0));
        return;
    }

    // Left Boundary, velocity inlet (equilibrium), a short sinusodial kick seeds the vortex shedding
    if (pos.x == 0) {
        vec2 inletVel = vec2(pc.inletVelocity, 0.0);
        if (pc.stepIndex < 500U) {
            inletVel.y = 0.01 * pc.inletVelocity * sin(float(pos.y) * 0.15 + float(pc.stepIndex) * 0.05);
        }
        writeEquilibrium(cell, 1.0, inletVel);
        return;
    }

    // Right Boundary, convective outflow via zeroth-order extrapolation from the previous column
    if (pos.x == int(pc.gridWidth) - 1) {
        uint sourceCell = cellIndex(ivec2(pos.x - 1, pos.y));
        for (uint q = 0U; q < 9U; ++q) {
            fOut[fIndex(q, cell)] = fIn[fIndex(q, sourceCell)];
        }
        return;
    }

    // Streaming, gather from the upstream neighbor of each direction
    float f[9];
    for (uint q = 0U; q < 9U; ++q) {
        ivec2 source = pos - C[q];

        if (source.y < 0 || source.y >= int(pc.gridHeight)) {
            f[q] = fIn[fIndex(uint(kMirrorY[q]), cell)];
        } else if (obstacle[cellIndex(source)] != 0U) {
            f[q] = fIn[fIndex(uint(kOpposite[q]), cell)];
        } else {
            f[q] = fIn[fIndex(q, cellIndex(source))];
        }
    }

    // Macroscopic density and velocity
    float rho = 0.0;
    vec2 momentum = vec2(0.0);
    for (uint q = 0U; q < 9U; ++q) {
        rho += f[q];
        momentum += f[q] * vec2(C[q]);
    }
    vec2 u = momentum / max(rho, 1e-5);

    // BGK/SRT Collision
    float omega = 1.0 / pc.relaxationTime;
    for (uint q = 0U; q < 9U; ++q) {
        fOut[fIndex(q, cell)] = f[q] + omega * (equilibrium(q, rho, u) - f[q]);
    }
}
