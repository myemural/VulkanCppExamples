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
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float uvScale;
    int diffuseMap;
    int normalMap;
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 pointLightPosition;          // xyz = Light Position
    vec4 pointLightColor;             // xyz = Light Color
    vec4 pointLightParams;            // x = Constant Factor, y = Linear Factor, z = Quadratic Factor

    vec4 directionalLightDirection;   // xyz = Light Direction (world space, normalized)
    vec4 directionalLightColor;       // xyz = Light Color
    mat4 directionalLightSpaceMatrix; // Light-space matrix for directional light

    vec4 spotLightPosition;           // xyz = Light Position
    vec4 spotLightDirection;          // xyz = Light Direction
    vec4 spotLightColor;              // xyz = Light Color
    vec4 spotLightParams;             // x = cos(innerCutoffAngle), y = cos(outerCutoffAngle)
    mat4 spotLightSpaceMatrix;        // Light-space matrix for spotlight
} light;

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

layout(set = 0, binding = 4) uniform samplerCube uPointLightCubemapShadowMap;

layout(set = 0, binding = 5) uniform sampler2D uDirLightShadowMap;

layout(set = 0, binding = 6) uniform sampler2D uSpotLightShadowMap;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    float farPlane;
} pc;

void main()
{
    // Hardcoded white color for light objects
    outColor = vec4(1.0, 1.0, 1.0, 1.0);
}