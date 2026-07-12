#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout (triangles) in;
layout (line_strip, max_vertices = 4) out;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    uint objectId;
} pc;

void main()
{
    // Draw the vertices as they are
    gl_Position = pc.proj * gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = pc.proj * gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = pc.proj * gl_in[2].gl_Position;
    EmitVertex();
    gl_Position = pc.proj * gl_in[0].gl_Position; // For closing triangle
    EmitVertex();

    EndPrimitive();
}