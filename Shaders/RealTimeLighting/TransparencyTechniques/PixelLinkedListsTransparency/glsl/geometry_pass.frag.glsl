#version 450
#extension GL_EXT_nonuniform_qualifier : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define NULL_NODE 0xFFFFFFFFu
#define MAX_FRAGMENTS 8U

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in mat3 fragTBN;

struct MeshMaterialData
{
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float opacity;
    int diffuseMap;
    int normalMap;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightDirection; // xyz = Light Direction
    vec4 lightColor;     // rgb = Light Color
} light;

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

struct OitNode
{
    uint colorPacked;
    float depth;
    uint next;
};

layout(std430, set = 0, binding = 4) buffer OitLinkedListBuffer
{
    OitNode nodes[];
};

layout(std430, set=0, binding = 5) buffer OitHeadPointerBuffer
{
    uint heads[];
};

layout(std430, set=0, binding = 6) buffer OitAtomicCounterBuffer
{
    uint nodeCount;
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    uint screenWidth;
    uint screenHeight;
} pc;

void main()
{
    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    vec3 diffuseColor = meshMatInfo.diffuseColor.rgb;
    if (meshMatInfo.diffuseMap != -1) {
        vec4 diffuseTextureColor = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.diffuseMap)], fragUv);
        diffuseColor = diffuseTextureColor.rgb;
    }

    // Normal map calculation
    vec3 normalWorldSpace;
    if (meshMatInfo.normalMap != -1) {
        // Normal map sample (tangent space)
        vec3 normalTangent = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.normalMap)], fragUv).rgb;

        // Transform from [0,1] to [-1,1] range
        normalTangent = normalize(normalTangent * 2.0 - 1.0);

        // Tangent space to world space
        normalWorldSpace = normalize(fragTBN * normalTangent);
    }
    else
    {
        // Fallback: Geometric normal
        normalWorldSpace = normalize(fragTBN[2]);
    }

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(-light.lightDirection.xyz);

    // Normalizing view direction (camera position)
    vec3 normalizedView = normalize(pc.cameraPosition.xyz - fragPos);

    // Ambient calculation
    vec3 ambient = meshMatInfo.ambientStrength * diffuseColor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalWorldSpace, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * diffuseColor;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalWorldSpace, halfDir), 0.0), meshMatInfo.shininess);
    vec3 specular = meshMatInfo.specularStrength * spec * light.lightColor.rgb * meshMatInfo.specularColor.rgb;

    // Final color
    vec3 finalColor = ambient + diffuse + specular;
    vec4 color = vec4(finalColor, meshMatInfo.opacity);

    // Discard full transparent fragments for optimization
    if (color.a < 0.01) {
        discard;
    }

    // Get new node index
    uint nodeIndex = atomicAdd(nodeCount, 1U);

    // If buffer full, discard the fragment
    if (nodeIndex >= MAX_FRAGMENTS * pc.screenWidth * pc.screenHeight) {
        return;
    }

    // Calculate pixel index
    ivec2 coords = ivec2(gl_FragCoord.xy);
    uint pixelIdx = uint(coords.y) * pc.screenWidth + uint(coords.x);

    // Pack the color and fill the node
    nodes[nodeIndex].colorPacked = packUnorm4x8(color);
    nodes[nodeIndex].depth = gl_FragCoord.z;
    nodes[nodeIndex].next = atomicExchange(heads[pixelIdx], nodeIndex);
}