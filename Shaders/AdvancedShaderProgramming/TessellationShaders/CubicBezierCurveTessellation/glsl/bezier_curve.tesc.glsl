#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(vertices = 4) out;

layout(push_constant) uniform MeshPushConstants {
    vec4 shapeColor; // xyz = Color for control point or Bézier curve
    float controlPointSize;
    float tessLevel;
} pc;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 1.0; // Only 1 isoline required
        gl_TessLevelOuter[1] = pc.tessLevel; // Curve along segment count
    }
}
