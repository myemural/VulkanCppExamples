#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout (points) in;
layout (triangle_strip, max_vertices = 36) out;

layout(location = 0) in vec3 worldPosIn[];

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

const float HALF_SIZE = 0.1;

const vec3 cubePoints[8] = vec3[]
(
    vec3(-1, -1, -1),
    vec3(1, -1, -1),
    vec3(1, 1, -1),
    vec3(-1, 1, -1),
    vec3(-1, -1, 1),
    vec3(1, -1, 1),
    vec3(1, 1, 1),
    vec3(-1, 1, 1)
);

void EmitTriangle(vec3 a, vec3 b, vec3 c, vec3 normal)
{
    vec3 center = worldPosIn[0];

    fragNormal = normal;

    fragPos = center + a * HALF_SIZE;
    gl_Position = pc.proj * pc.view * vec4(fragPos, 1.0);
    EmitVertex();

    fragPos = center + b * HALF_SIZE;
    gl_Position = pc.proj * pc.view * vec4(fragPos, 1.0);
    EmitVertex();

    fragPos = center + c * HALF_SIZE;
    gl_Position = pc.proj * pc.view * vec4(fragPos, 1.0);
    EmitVertex();

    EndPrimitive();
}

void main()
{
    // Front Face
    EmitTriangle(cubePoints[4], cubePoints[5], cubePoints[6], vec3(0, 0, 1));
    EmitTriangle(cubePoints[4], cubePoints[6], cubePoints[7], vec3(0, 0, 1));

    // Back Face
    EmitTriangle(cubePoints[1], cubePoints[0], cubePoints[3], vec3(0, 0, -1));
    EmitTriangle(cubePoints[1], cubePoints[3], cubePoints[2], vec3(0, 0, -1));

    // Left Face
    EmitTriangle(cubePoints[0], cubePoints[4], cubePoints[7], vec3(-1, 0, 0));
    EmitTriangle(cubePoints[0], cubePoints[7], cubePoints[3], vec3(-1, 0, 0));

    // Right Face
    EmitTriangle(cubePoints[5], cubePoints[1], cubePoints[2], vec3(1, 0, 0));
    EmitTriangle(cubePoints[5], cubePoints[2], cubePoints[6], vec3(1, 0, 0));

    // Top Face
    EmitTriangle(cubePoints[3], cubePoints[7], cubePoints[6], vec3(0, 1, 0));
    EmitTriangle(cubePoints[3], cubePoints[6], cubePoints[2], vec3(0, 1, 0));

    // Bottom Face
    EmitTriangle(cubePoints[0], cubePoints[1], cubePoints[5], vec3(0, -1, 0));
    EmitTriangle(cubePoints[0], cubePoints[5], cubePoints[4], vec3(0, -1, 0));
}