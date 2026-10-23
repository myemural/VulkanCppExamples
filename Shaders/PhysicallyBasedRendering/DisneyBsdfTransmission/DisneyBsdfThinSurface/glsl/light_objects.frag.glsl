#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in mat3 fragTBN;

struct MeshMaterialData
{
    vec4 albedoColor;
    vec4 transmittanceColor; // rgb = Volumetric transmittance color, a = Unused
    float roughness;
    float metallic;
    float specularTint;
    float anisotropic;
    float sheen;
    float sheenTint;
    float clearcoat;
    float clearcoatGloss;
    float ior;
    float specTrans; // Specular Transmission
    float atDistance; // Distance at which transmittanceColor is reached
    float thickness; // Estimated path length of the refracted ray
    float diffTrans; // Thin surface parameter, range: [0, 2]
    float flatness; // Thin surface parameter, range: [0, 1]
    float uvScale;
    int albedoMap;
    int roughnessMap;
    int metallicMap;
    int normalMap;
    int thinSurface; // 0: Solid, 1: Thin Surface
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    uint prefilterRoughnessMaxMipCount;
    uint isIblEnabled; // 0: IBL Disabled, 1: IBL Enabled
    uint isToneMapEnabled;
} pc;

// The light objects are drawn into the linear HDR scene color, so they stay bright after tone mapping
const float kLightObjectBrightness = 0.6f;

void main()
{
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];
    outColor = vec4(meshInfo.albedoColor.rgb * kLightObjectBrightness, 1.0);
}
