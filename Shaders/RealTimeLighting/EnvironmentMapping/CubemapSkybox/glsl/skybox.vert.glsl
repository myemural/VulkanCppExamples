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

layout(location = 0) out vec3 directionVector;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

void main()
{
    // Eliminate translation from view matrix
    mat3 viewNoTranslation = mat3(pc.view);

    // Vertex position to view direction
    vec3 worldPos = transpose(viewNoTranslation) * inPosition;
    directionVector = worldPos;

    // Z = W
    vec4 clipPos = pc.proj * vec4(inPosition, 1.0);
    gl_Position = clipPos.xyww;
}