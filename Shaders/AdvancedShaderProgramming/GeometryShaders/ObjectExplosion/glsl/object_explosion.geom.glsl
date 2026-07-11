#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 fragPosIn[];
layout(location = 1) in vec2 fragUvIn[];
layout(location = 2) in mat3 fragTBNIn[];

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out mat3 fragTBN;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    float time;
} pc;


vec3 ExplodeInWorldSpace(vec3 position, vec3 normal)
{
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(pc.time) + 1.0) / 2.0) * magnitude;
    return position + direction;
}

vec3 GetNormal()
{
    vec3 a = fragPosIn[0] - fragPosIn[1];
    vec3 b = fragPosIn[2] - fragPosIn[1];
    return normalize(cross(b, a));
}

void main()
{
    vec3 normal = GetNormal();

    // Explode in world space first, then project to clip space
    vec3 explodedPos0 = ExplodeInWorldSpace(fragPosIn[0], normal);
    vec3 explodedPos1 = ExplodeInWorldSpace(fragPosIn[1], normal);
    vec3 explodedPos2 = ExplodeInWorldSpace(fragPosIn[2], normal);

    mat4 viewProj = pc.proj * pc.view;

    gl_Position = viewProj * vec4(explodedPos0, 1.0);
    fragPos = explodedPos0;
    fragUv = fragUvIn[0];
    fragTBN = fragTBNIn[0];
    EmitVertex();

    gl_Position = viewProj * vec4(explodedPos1, 1.0);
    fragPos = explodedPos1;
    fragUv = fragUvIn[1];
    fragTBN = fragTBNIn[1];
    EmitVertex();

    gl_Position = viewProj * vec4(explodedPos2, 1.0);
    fragPos = explodedPos2;
    fragUv = fragUvIn[2];
    fragTBN = fragTBNIn[2];
    EmitVertex();

    EndPrimitive();
}