#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define RECT_AREA_LIGHT_COUNT 4U
#define SPHERE_AREA_LIGHT_COUNT 2U
#define DIRECTIONAL_LIGHT_COUNT 1U

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in mat3 fragTBN;

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

struct RectangularAreaLightData
{
    vec4 lightColorAndIntensity; // xyz = Light Color, w = Light Intensity
    vec4 lightRectCornerPos[4];  // xyz = Light Rectangle Corner Position
    int isDoubleSided;           // 0: Single-sided, 1: Double-sided
    float _pad[3];
};

struct SphereAreaLightData
{
    vec4 lightPositionAndRadius; // xyz = Light Position, w = Light Radius
    vec4 lightColorAndIntensity; // xyz = Light Color, w = Light Intensity
};

struct DirectionalLightData
{
    vec4 lightDirection;         // xyz = Light Direction
    vec4 lightColorAndIntensity; // xyz = Light Color, w = Light Intensity
};

layout(std430, set = 0, binding = 2) readonly buffer LightBuffer
{
    RectangularAreaLightData rectangularAreaLights[RECT_AREA_LIGHT_COUNT];
    SphereAreaLightData sphereAreaLights[SPHERE_AREA_LIGHT_COUNT];
    DirectionalLightData directionalLights[DIRECTIONAL_LIGHT_COUNT];
};

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];
layout(set = 0, binding = 4) uniform sampler2D uLtc1;
layout(set = 0, binding = 5) uniform sampler2D uLtc2;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

void main()
{
    // Get mesh material info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    // Apply only albedo color for light objects
    outColor = meshMatInfo.albedoColor;
}