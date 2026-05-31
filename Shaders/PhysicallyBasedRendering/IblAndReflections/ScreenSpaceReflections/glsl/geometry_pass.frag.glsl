#version 450
#extension GL_EXT_nonuniform_qualifier : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gAlbedo;
layout(location = 2) out vec4 gMetallicRoughness;
layout(location = 3) out vec4 gNormal;

layout(location = 0) in vec3 fragPosView;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in mat3 fragTbnView;

struct MeshMaterialData
{
    vec4 albedoColor;
    float roughness;
    float metallic;
    float uvScale;
    int albedoMap;
    int roughnessMap;
    int metallicMap;
    int normalMap;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(set = 0, binding = 2) uniform sampler2D uCombinedSamplers[];

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    uint objectId;
} pc;

void main()
{
    // Get mesh info
    const MeshMaterialData meshMaterial = meshMaterials[pc.objectId];

    // Calculate scaled UV
    const vec2 scaledUv = fragUv * meshMaterial.uvScale;

    gPosition = vec4(fragPosView, 1.0);

    vec4 albedo = meshMaterial.albedoColor.rgba;
    if (meshMaterial.albedoMap != -1) {
        albedo = texture(uCombinedSamplers[nonuniformEXT(meshMaterial.albedoMap)], scaledUv);
    }

    gAlbedo = albedo;

    // Normal map calculation
    vec3 normalViewSpace;
    if (meshMaterial.normalMap != -1) {
        // Normal map sample (tangent space)
        vec3 normalTangent = texture(uCombinedSamplers[nonuniformEXT(meshMaterial.normalMap)], scaledUv).rgb;

        // Transform from [0,1] to [-1,1] range
        normalTangent = normalize(normalTangent * 2.0 - 1.0);

        // Tangent space to view space
        normalViewSpace = normalize(fragTbnView * normalTangent);
    }
    else
    {
        // Fallback: Geometric normal
        normalViewSpace = normalize(fragTbnView[2]);
    }

    // No need to pack normal, because already using VK_FORMAT_R16G16B16A16_SFLOAT
    gNormal = vec4(normalViewSpace, 1.0);

    float metallic = meshMaterial.metallic;
    if (meshMaterial.metallicMap != -1) {
        metallic = texture(uCombinedSamplers[nonuniformEXT(meshMaterial.metallicMap)], scaledUv).r;
    }
    gMetallicRoughness.r = metallic;


    float roughness = meshMaterial.roughness;
    if (meshMaterial.roughnessMap != -1) {
        roughness = texture(uCombinedSamplers[nonuniformEXT(meshMaterial.roughnessMap)], scaledUv).r;
    }
    roughness = clamp(roughness, 0.04, 1.0);
    gMetallicRoughness.g = roughness;
}