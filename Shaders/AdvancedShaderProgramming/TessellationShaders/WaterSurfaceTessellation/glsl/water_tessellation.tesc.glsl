#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(vertices = 4) out;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec2 inUv[];
layout(location = 2) in vec3 inNormal[];

layout(location = 0) out vec3 outPosition[];
layout(location = 1) out vec2 outUv[];
layout(location = 2) out vec3 outNormal[];

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    float tessellationLevel;
    float displacementLevel;
    float time;
} pc;

void main()
{
    // Transfer each control point to the Tessellation Evaluation Shader as is
    outPosition[gl_InvocationID] = inPosition[gl_InvocationID];
    outUv[gl_InvocationID] = inUv[gl_InvocationID];
    outNormal[gl_InvocationID] = inNormal[gl_InvocationID];

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = pc.tessellationLevel;
        gl_TessLevelOuter[1] = pc.tessellationLevel;
        gl_TessLevelOuter[2] = pc.tessellationLevel;
        gl_TessLevelOuter[3] = pc.tessellationLevel;

        gl_TessLevelInner[0] = pc.tessellationLevel;
        gl_TessLevelInner[1] = pc.tessellationLevel;
    }
}
