#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(vertices = 3) out;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec2 inUV[];
layout(location = 2) in vec3 inNormal[];
layout(location = 3) in vec4 inTangent[];

layout(location = 0) out vec3 outPosition[3];
layout(location = 1) out vec2 outUV[3];
layout(location = 2) out vec3 outNormal[3];
layout(location = 3) out vec4 outTangent[3];

// PN-Triangle cubic Bezier checkpoints
layout(location = 4)  patch out vec3 patchB210;
layout(location = 5)  patch out vec3 patchB120;
layout(location = 6)  patch out vec3 patchB021;
layout(location = 7)  patch out vec3 patchB012;
layout(location = 8)  patch out vec3 patchB102;
layout(location = 9)  patch out vec3 patchB201;
layout(location = 10) patch out vec3 patchB111;

// Quadratic normal-patch checkpoints
layout(location = 11) patch out vec3 patchN110;
layout(location = 12) patch out vec3 patchN011;
layout(location = 13) patch out vec3 patchN101;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    float tessLevel;
    float tessAlpha;
} pc;

float wij(vec3 pi, vec3 pj, vec3 ni)
{
    return dot(pj - pi, ni);
}

vec3 edgeControlPoint(vec3 pi, vec3 pj, vec3 ni)
{
    return (2.0 * pi + pj - wij(pi, pj, ni) * ni) / 3.0;
}

vec3 normalControlPoint(vec3 pi, vec3 pj, vec3 ni, vec3 nj)
{
    vec3 edge = pj - pi;
    float vij = 2.0 * dot(edge, ni + nj) / max(dot(edge, edge), 1e-8);
    return normalize(ni + nj - vij * edge);
}

void main()
{
    outPosition[gl_InvocationID] = inPosition[gl_InvocationID];
    outUV[gl_InvocationID] = inUV[gl_InvocationID];
    outNormal[gl_InvocationID] = inNormal[gl_InvocationID];
    outTangent[gl_InvocationID] = inTangent[gl_InvocationID];

    // Tessellation levels
    gl_TessLevelInner[0] = pc.tessLevel;
    gl_TessLevelOuter[0] = pc.tessLevel;
    gl_TessLevelOuter[1] = pc.tessLevel;
    gl_TessLevelOuter[2] = pc.tessLevel;

    // Patch constants (10 Bezier + 6 normal checkpoints) are calculated only once
    if (gl_InvocationID == 0)
    {
        vec3 p1 = inPosition[0];
        vec3 p2 = inPosition[1];
        vec3 p3 = inPosition[2];

        vec3 n1 = inNormal[0];
        vec3 n2 = inNormal[1];
        vec3 n3 = inNormal[2];

        patchB210 = edgeControlPoint(p1, p2, n1);
        patchB120 = edgeControlPoint(p2, p1, n2);
        patchB021 = edgeControlPoint(p2, p3, n2);
        patchB012 = edgeControlPoint(p3, p2, n3);
        patchB102 = edgeControlPoint(p3, p1, n3);
        patchB201 = edgeControlPoint(p1, p3, n1);

        vec3 centerEdge = (patchB210 + patchB120 + patchB021 + patchB012 + patchB102 + patchB201) / 6.0;
        vec3 centroidPoint = (p1 + p2 + p3) / 3.0;
        patchB111 = centerEdge + (centerEdge - centroidPoint) * 0.5;

        patchN110 = normalControlPoint(p1, p2, n1, n2);
        patchN011 = normalControlPoint(p2, p3, n2, n3);
        patchN101 = normalControlPoint(p3, p1, n3, n1);
    }
}
