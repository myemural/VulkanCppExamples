#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

layout(std430, binding = 0) buffer Distribution { float f[]; };
layout(std430, binding = 2) buffer Dye { float dye[]; };
layout(std430, binding = 4) buffer ObstacleMask { uint obstacle[]; };
layout(binding = 5, rgba8) uniform writeonly image2D destImage;

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

uint cellIndex(ivec2 p)
{
    return uint(p.y) * pc.gridWidth + uint(p.x);
}

uint fIndex(uint q, uint cell)
{
    return q * (pc.gridWidth * pc.gridHeight) + cell;
}

// Density and velocity of one cell
void macroscopic(ivec2 p, out float rho, out vec2 u)
{
    p = clamp(p, ivec2(0), ivec2(int(pc.gridWidth - 1), int(pc.gridHeight - 1)));
    uint cell = cellIndex(p);

    if (obstacle[cell] != 0U) {
        rho = 1.0;
        u = vec2(0.0);
        return;
    }

    rho = 0.0;
    vec2 momentum = vec2(0.0);
    for (uint q = 0U; q < 9U; ++q) {
        float fq = f[fIndex(q, cell)];
        rho += fq;
        momentum += fq * vec2(C[q]);
    }
    u = momentum / max(rho, 1e-5);
}

// Turbo-like sequential colormap
vec3 turbo(float t)
{
    const vec3 stops[5] = vec3[5](vec3(0.19, 0.07, 0.23), vec3(0.1, 0.55, 0.85), vec3(0.2, 0.85, 0.45),
                                  vec3(0.95, 0.85, 0.2), vec3(0.75, 0.12, 0.08));
    t = clamp(t, 0.0, 1.0) * 4.0;
    int i = min(int(t), 3);
    return mix(stops[i], stops[i + 1], t - float(i));
}

// Blue-white-red diverging colormap
vec3 diverging(float s)
{
    return s < 0.0 ? mix(vec3(1.0), vec3(0.15, 0.3, 0.9), -s) : mix(vec3(1.0), vec3(0.9, 0.15, 0.1), s);
}

void main()
{
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    if (pos.x >= int(pc.gridWidth) || pos.y >= int(pc.gridHeight)) {
        return;
    }

    uint cell = cellIndex(pos);

    if (obstacle[cell] != 0U) {
        imageStore(destImage, pos, vec4(vec3(0.25), 1.0));
        return;
    }

    float rho;
    vec2 u;
    macroscopic(pos, rho, u);

    // Blow ups become visible instead of turning the screen black
    if (isnan(rho) || isinf(rho)) {
        imageStore(destImage, pos, vec4(1.0, 0.0, 1.0, 1.0));
        return;
    }

    vec3 color;

    if (pc.displayMode == 2U) {
        // Vorticity: Curl of the velocity field via central differences
        float rhoTmp;
        vec2 uLeft, uRight, uDown, uUp;
        macroscopic(pos + ivec2(-1, 0), rhoTmp, uLeft);
        macroscopic(pos + ivec2(1, 0), rhoTmp, uRight);
        macroscopic(pos + ivec2(0, -1), rhoTmp, uDown);
        macroscopic(pos + ivec2(0, 1), rhoTmp, uUp);

        float vorticity = 0.5 * ((uRight.y - uLeft.y) - (uUp.x - uDown.x));
        color = diverging(clamp(vorticity * 50.0, -1.0, 1.0));
    } else if (pc.displayMode == 3U) {
        // Density: Proportional to pressure in LBM
        color = diverging(clamp((rho - 1.0) * 100.0, -1.0, 1.0));
    } else if (pc.displayMode == 4U) {
        // Dye
        color = mix(vec3(0.02, 0.03, 0.06), vec3(1.0), clamp(dye[cell], 0.0, 1.0));
    } else {
        // Velocity Magnitude
        color = turbo(length(u) / (1.5 * pc.inletVelocity));
    }

    imageStore(destImage, pos, vec4(color, 1.0));
}
