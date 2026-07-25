#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(vertices = 16) out;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    float tessLevel;
} pc;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = pc.tessLevel;
        gl_TessLevelOuter[1] = pc.tessLevel;
        gl_TessLevelOuter[2] = pc.tessLevel;
        gl_TessLevelOuter[3] = pc.tessLevel;

        gl_TessLevelInner[0] = pc.tessLevel;
        gl_TessLevelInner[1] = pc.tessLevel;
    }
}
