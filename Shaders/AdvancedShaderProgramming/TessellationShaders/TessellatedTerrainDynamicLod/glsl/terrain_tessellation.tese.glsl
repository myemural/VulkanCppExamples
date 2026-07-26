#version 450
#extension GL_EXT_nonuniform_qualifier : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(quads, fractional_odd_spacing, ccw) in;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec2 inUv[];
layout(location = 2) in vec3 inNormal[];
layout(location = 3) in vec4 inTangent[];

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out mat3 fragTBN;
layout(location = 5) out vec2 fragPatchCoord;

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
    float uvScale;
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
    uint enablePatchLines; // 0 = False, 1 = True
    float displacementLevel;
    float minTessDistance;
    float maxTessDistance;
    float minTessLevel;
    float maxTessLevel;
} pc;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2, vec2 v3)
{
    vec2 bottom = mix(v0, v1, gl_TessCoord.x);
    vec2 top = mix(v3, v2, gl_TessCoord.x);
    return mix(bottom, top, gl_TessCoord.y);
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2, vec3 v3)
{
    vec3 bottom = mix(v0, v1, gl_TessCoord.x);
    vec3 top = mix(v3, v2, gl_TessCoord.x);
    return mix(bottom, top, gl_TessCoord.y);
}

vec4 interpolate4D(vec4 v0, vec4 v1, vec4 v2, vec4 v3)
{
    vec4 bottom = mix(v0, v1, gl_TessCoord.x);
    vec4 top = mix(v3, v2, gl_TessCoord.x);
    return mix(bottom, top, gl_TessCoord.y);
}

float sampleHeight(vec2 uv, int heightMap)
{
    if (heightMap == -1) {
        return 0.0;
    }
    return texture(uCombinedSamplers[nonuniformEXT(heightMap)], uv).r;
}

void main()
{
    const MeshTransformData meshTransform = meshTransforms[pc.objectId];
    const MeshMaterialData meshMaterial = meshMaterials[pc.objectId];

    // Object-space bilinear interpolation across the quad patch
    vec3 position = interpolate3D(inPosition[0], inPosition[1], inPosition[2], inPosition[3]);
    vec2 uv = interpolate2D(inUv[0], inUv[1], inUv[2], inUv[3]);
    vec4 tangent = interpolate4D(inTangent[0], inTangent[1], inTangent[2], inTangent[3]);

    // Sample displacement (object space) at the interpolated UV
    float height = sampleHeight(uv, meshMaterial.heightMap);

    // Displace vertex along the plane's fixed up-axis (object-space Y)
    vec3 displacedPosition = position + vec3(0.0, 1.0, 0.0) * height * pc.displacementLevel;

    // Texel size in UV space, used for the finite-difference normal reconstruction below
    vec2 texelSize = vec2(1.0);
    if (meshMaterial.heightMap != -1) {
        texelSize = 1.0 / vec2(textureSize(uCombinedSamplers[nonuniformEXT(meshMaterial.heightMap)], 0));
    }

    // Reconstruct the normal from the heightmap via finite differences (Sobel-style
    // central difference), since the source mesh's per-vertex normal is a constant
    // (0,1,0) and cannot capture slope introduced by displacement.
    vec3 normal = vec3(0.0, 1.0, 0.0);
    if (meshMaterial.heightMap != -1) {
        float hL = sampleHeight(uv - vec2(texelSize.x, 0.0), meshMaterial.heightMap);
        float hR = sampleHeight(uv + vec2(texelSize.x, 0.0), meshMaterial.heightMap);
        float hD = sampleHeight(uv - vec2(0.0, texelSize.y), meshMaterial.heightMap);
        float hU = sampleHeight(uv + vec2(0.0, texelSize.y), meshMaterial.heightMap);

        // Slope-to-normal conversion; displacementLevel scales the height contribution
        // so steeper displacement produces steeper normals.
        float sx = (hL - hR) / (2.0 * texelSize.x) * pc.displacementLevel;
        float sz = (hD - hU) / (2.0 * texelSize.y) * pc.displacementLevel;
        normal = normalize(vec3(sx, 1.0, sz));
    }

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

    fragPatchCoord = gl_TessCoord.xy;
    gl_Position = pc.proj * pc.view * worldPos;
}
