#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

layout(location = 0) in vec3 inFragPos[];

layout(location = 0) out vec3 outFragPos;

layout(set = 0, binding = 1) uniform LightSpaceMatricesUBO {
    mat4 lightSpaceMatrices[6]; // For each cubemap face
} lsm;

void main()
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for (int i = 0; i < 3; ++i)
        {
            outFragPos = inFragPos[i];
            gl_Position = lsm.lightSpaceMatrices[face] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}