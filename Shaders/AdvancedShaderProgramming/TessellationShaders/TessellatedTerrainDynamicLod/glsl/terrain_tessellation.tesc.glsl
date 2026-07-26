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
layout(location = 3) in vec4 inTangent[];

layout(location = 0) out vec3 outPosition[];
layout(location = 1) out vec2 outUv[];
layout(location = 2) out vec3 outNormal[];
layout(location = 3) out vec4 outTangent[];

struct MeshTransformData
{
    mat4 model;
    mat4 normalMatrix;
};

layout(std430, binding = 0) readonly buffer MeshTransformDataBuffer {
    MeshTransformData meshTransforms[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    uint enablePatchLines; // 0 = False, 1 = True
    float displacementLevel;
    float minTessDistance;
    float maxTessDistance;
    float minTessLevel;
    float maxTessLevel;
} pc;

// Symmetric (order-independent) per-edge tessellation factor.
float edgeTessLevel(vec3 worldA, vec3 worldB)
{
    float distA = distance(pc.cameraPosition.xyz, worldA);
    float distB = distance(pc.cameraPosition.xyz, worldB);
    float d = (distA + distB) * 0.5; // It is symmetric, so order of A/B does not matter

    float range = max(pc.maxTessDistance - pc.minTessDistance, 0.0001);
    float t = clamp((d - pc.minTessDistance) / range, 0.0, 1.0);
    float level = mix(pc.maxTessLevel, pc.minTessLevel, t);

    // 64 matches the common VkPhysicalDeviceLimits::maxTessellationGenerationLevel (in most cases)
    return clamp(level, 1.0, 64.0);
}

void main()
{
    // Transfer each control point to the Tessellation Evaluation Shader as is
    outPosition[gl_InvocationID] = inPosition[gl_InvocationID];
    outUv[gl_InvocationID] = inUv[gl_InvocationID];
    outNormal[gl_InvocationID] = inNormal[gl_InvocationID];
    outTangent[gl_InvocationID] = inTangent[gl_InvocationID];

    if (gl_InvocationID == 0) {
        const mat4 model = meshTransforms[pc.objectId].model;

        // World-space corners. inPosition[] holds all 4 control points of the
        // patch regardless of gl_InvocationID, so this is safe to do once
        vec3 w0 = (model * vec4(inPosition[0], 1.0)).xyz;
        vec3 w1 = (model * vec4(inPosition[1], 1.0)).xyz;
        vec3 w2 = (model * vec4(inPosition[2], 1.0)).xyz;
        vec3 w3 = (model * vec4(inPosition[3], 1.0)).xyz;

        // Quad domain edge convention
        float e0 = edgeTessLevel(w0, w3);
        float e1 = edgeTessLevel(w0, w1);
        float e2 = edgeTessLevel(w1, w2);
        float e3 = edgeTessLevel(w3, w2);

        gl_TessLevelOuter[0] = e0;
        gl_TessLevelOuter[1] = e1;
        gl_TessLevelOuter[2] = e2;
        gl_TessLevelOuter[3] = e3;

        // Inner levels derived from the opposing outer edges
        gl_TessLevelInner[0] = (e1 + e3) * 0.5;
        gl_TessLevelInner[1] = (e0 + e2) * 0.5;
    }
}
