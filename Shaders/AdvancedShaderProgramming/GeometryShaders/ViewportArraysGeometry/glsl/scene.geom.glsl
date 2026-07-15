#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout (triangles) in;
layout (triangle_strip, max_vertices = 12) out;

layout(location = 0) in vec3 inFragPos[];
layout(location = 1) in vec2 inFragUv[];
layout(location = 2) in mat3 inFragTBN[];

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out mat3 fragTBN;
layout(location = 5) flat out int outViewportIndex;

void main()
{
    for(int viewport = 0; viewport < 4; viewport++)
    {
        gl_ViewportIndex = viewport;
        outViewportIndex = viewport;

        for(int i = 0; i < 3; i++)
        {
            fragPos = inFragPos[i];
            fragUv = inFragUv[i];
            fragTBN = inFragTBN[i];
            gl_Position = gl_in[i].gl_Position;
            EmitVertex();
        }

        EndPrimitive();
    }
}