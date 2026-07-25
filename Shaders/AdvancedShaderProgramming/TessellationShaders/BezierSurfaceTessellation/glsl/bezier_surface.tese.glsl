#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define GRID_SIZE 4 // Coming from 4x4=16 control points

layout (quads, equal_spacing, ccw) in;

layout(location = 0) out vec2 fragUv;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    float tessLevel;
} pc;

// Cubic Bernstein basis functions B0..B3(t)
vec4 bernsteinBasis(float t)
{
    float invT = 1.0 - t;
    return vec4(
            invT * invT * invT,        // B0
            3.0 * t * invT * invT,     // B1
            3.0 * t * t * invT,        // B2
            t * t * t                  // B3
    );
}

// Control point layout convention: row-major 4x4 grid
vec3 controlPoint(int row, int col)
{
    return gl_in[row * GRID_SIZE + col].gl_Position.xyz;
}

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec4 bu = bernsteinBasis(u);
    vec4 bv = bernsteinBasis(v);

    vec3 position = vec3(0.0);
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            position += controlPoint(row, col) * (bu[col] * bv[row]);
        }
    }

    fragUv = vec2(u, v);
    gl_Position = pc.proj * pc.view * vec4(position, 1.0);
}
