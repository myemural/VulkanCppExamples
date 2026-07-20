#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(vertices = 3) out;

layout(push_constant) uniform MeshPushConstants {
    float triangleInnerTessLevel;
} pc;

void main()
{
    // Transfer each control point to the Tessellation Evaluation Shader as is
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // You only need to adjust the tessellation levels once
    if (gl_InvocationID == 0)
    {
        // Outer: How many parts each side of the triangle will be divided into
        gl_TessLevelOuter[0] = 8.0;
        gl_TessLevelOuter[1] = 8.0;
        gl_TessLevelOuter[2] = 8.0;

        // Inner: How dense the tessellation will be in the interior of the triangle
        gl_TessLevelInner[0] = pc.triangleInnerTessLevel;
    }
}
