#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

layout(location = 0) in vec3 geomNormal[];

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    uint objectId;
} pc;

const float LINE_LENGTH = 0.3;

void main()
{
    // Calculate centroid of the triangle in view-space
    vec4 centroidPos = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3.0;

    // Calculate average normal and normalize it
    vec3 avgNormal = normalize(geomNormal[0] + geomNormal[1] + geomNormal[2]);

    // Start point of the normal line
    gl_Position = pc.proj * centroidPos;
    EmitVertex();

    // End point of the normal line
    gl_Position = pc.proj * (centroidPos + vec4(avgNormal, 0.0) * LINE_LENGTH);
    EmitVertex();

    EndPrimitive();
}