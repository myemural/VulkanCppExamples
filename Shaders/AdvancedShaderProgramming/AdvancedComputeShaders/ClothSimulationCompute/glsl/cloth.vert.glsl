#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out vec3 fragNormal;

struct ClothVertex
{
    vec4 position; // xyz = World-Space Position
    vec4 normal;   // xyz = World-Space Normal
};

layout(std430, binding = 4) readonly buffer ClothVertexBuffer {
    ClothVertex clothVertices[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

void main()
{
    // Get new cloth vertex data
    const ClothVertex clothVertex = clothVertices[gl_VertexIndex];

    fragPos = clothVertex.position.xyz;
    fragNormal = clothVertex.normal.xyz;
    fragUv  = inUV;

    gl_Position = pc.proj * pc.view * vec4(fragPos, 1.0);
}
