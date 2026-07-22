#version 450
#extension GL_EXT_nonuniform_qualifier : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(triangles, fractional_odd_spacing, ccw) in;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec2 inUv[];
layout(location = 2) in vec3 inNormal[];
layout(location = 3) in vec4 inTangent[];

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out mat3 fragTBN;

struct MeshTransformData
{
    mat4 model;
    mat4 normalMatrix;
};
layout(std430, binding = 0) readonly buffer MeshTransformDataBuffer {
    MeshTransformData meshTransforms[];
};

struct MeshMaterialData
{
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    int diffuseMap;
    int normalMap;
    int heightMap;
};
layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    uint isSphericalObject; // 0: False, 1: True
    uint tessellationLevel;
    float displacementLevel;
} pc;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return v0 * gl_TessCoord.x + v1 * gl_TessCoord.y + v2 * gl_TessCoord.z;
}
vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return v0 * gl_TessCoord.x + v1 * gl_TessCoord.y + v2 * gl_TessCoord.z;
}
vec4 interpolate4D(vec4 v0, vec4 v1, vec4 v2)
{
    return v0 * gl_TessCoord.x + v1 * gl_TessCoord.y + v2 * gl_TessCoord.z;
}

void main()
{
    const MeshTransformData meshTransform = meshTransforms[pc.objectId];
    const MeshMaterialData meshMaterial = meshMaterials[pc.objectId];

    // Object-space barycentric interpolation
    vec3 position = interpolate3D(inPosition[0], inPosition[1], inPosition[2]);
    vec2 uv = interpolate2D(inUv[0], inUv[1], inUv[2]);
    vec3 normal = normalize(interpolate3D(inNormal[0], inNormal[1], inNormal[2]));
    vec4 tangent = interpolate4D(inTangent[0], inTangent[1], inTangent[2]);

    // Position and normal corrections which respect to spherical objects
    if (pc.isSphericalObject == 1U) {
        float radius = length(inPosition[0]); // Original radius of the base mesh
        vec3 sphereDir = normalize(position);
        position = sphereDir * radius;
        normal = sphereDir;
    }

    // Sample displacement (object space)
    float height = 0.0;
    if (meshMaterial.heightMap != -1) {
        height = texture(uCombinedSamplers[nonuniformEXT(meshMaterial.heightMap)], uv).r;
    }

    // Displace vertex along normal (object-space)
    vec3 displacedPosition = position + normal * height * pc.displacementLevel;

    // World-space position and UV
    vec4 worldPos = meshTransform.model * vec4(displacedPosition, 1.0);
    fragPos = worldPos.xyz;
    fragUv = uv;

    // World-space normal and tangent (TBN matrix generation)
    mat3 normalMatrix = mat3(meshTransform.normalMatrix);
    vec3 N = normalize(normalMatrix * normal);
    vec3 T = normalize(normalMatrix * tangent.xyz);
    T = normalize(T - N * dot(N, T));
    vec3 B = cross(N, T) * tangent.w;
    fragTBN = mat3(T, B, N);

    gl_Position = pc.proj * pc.view * worldPos;
}
