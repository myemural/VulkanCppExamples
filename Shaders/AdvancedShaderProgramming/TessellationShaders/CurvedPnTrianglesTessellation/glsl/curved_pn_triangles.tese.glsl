#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(triangles, fractional_odd_spacing, cw) in;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec2 inUV[];
layout(location = 2) in vec3 inNormal[];
layout(location = 3) in vec4 inTangent[];

layout(location = 4)  patch in vec3 patchB210;
layout(location = 5)  patch in vec3 patchB120;
layout(location = 6)  patch in vec3 patchB021;
layout(location = 7)  patch in vec3 patchB012;
layout(location = 8)  patch in vec3 patchB102;
layout(location = 9)  patch in vec3 patchB201;
layout(location = 10) patch in vec3 patchB111;

layout(location = 11) patch in vec3 patchN110;
layout(location = 12) patch in vec3 patchN011;
layout(location = 13) patch in vec3 patchN101;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out mat3 fragTBN;

struct MeshTransformData
{
    mat4 model;
    mat4 normalMatrix;
};

layout(std430, binding = 0) readonly buffer MeshTransformDataBuffer
{
    MeshTransformData meshTransforms[];
};

layout(push_constant) uniform MeshPushConstants
{
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    float tessLevel;
    float tessAlpha;
} pc;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = gl_TessCoord.z;

    // Position in model-space
    vec3 p1 = inPosition[0];
    vec3 p2 = inPosition[1];
    vec3 p3 = inPosition[2];

    vec3 flatPos = w * p1 + u * p2 + v * p3;

    float u2 = u * u, v2 = v * v, w2 = w * w;
    float u3 = u2 * u, v3 = v2 * v, w3 = w2 * w;

    vec3 curvedPos = p1 * w3 + p2 * u3 + p3 * v3 + patchB210 * 3.0 * w2 * u + patchB120 * 3.0 * w  * u2 +
                     patchB201 * 3.0 * w2 * v + patchB021 * 3.0 * u2 * v + patchB102 * 3.0 * w  * v2 +
                     patchB012 * 3.0 * u  * v2 + patchB111 * 6.0 * w  * u * v;

    vec3 modelPos = mix(flatPos, curvedPos, pc.tessAlpha);

    // Normal in model-space
    vec3 n1 = inNormal[0];
    vec3 n2 = inNormal[1];
    vec3 n3 = inNormal[2];

    vec3 flatNormal = w * n1 + u * n2 + v * n3;

    vec3 curvedNormal = n1 * w2 + n2 * u2 + n3 * v2 + patchN110 * w * u + patchN011 * u * v + patchN101 * w * v;
    vec3 modelNormal = normalize(mix(flatNormal, curvedNormal, pc.tessAlpha));

    // Tangent (in model-space, linear interpolation + orthonormalization)
    vec4 modelTangent4 = w * inTangent[0] + u * inTangent[1] + v * inTangent[2];
    vec3 modelTangent  = normalize(modelTangent4.xyz - modelNormal * dot(modelNormal, modelTangent4.xyz));

    // Model-space to world-space transformation
    const MeshTransformData meshTransform = meshTransforms[pc.objectId];
    mat3 normalMatrix = mat3(meshTransform.normalMatrix);

    vec3 worldPos = vec3(meshTransform.model * vec4(modelPos, 1.0));
    vec3 worldNormal = normalize(normalMatrix * modelNormal);
    vec3 worldTangent = normalize(normalMatrix * modelTangent);
    vec3 worldBitangent = cross(worldNormal, worldTangent) * modelTangent4.w;

    fragPos = worldPos;
    fragUv  = w * inUV[0] + u * inUV[1] + v * inUV[2];
    fragTBN = mat3(worldTangent, worldBitangent, worldNormal);
    gl_Position = pc.proj * pc.view * vec4(worldPos, 1.0);
}
