#version 450
#extension GL_EXT_nonuniform_qualifier : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3  fragNormal;
layout(location = 1) in vec3  fragWorldPos;
layout(location = 2) in float fragHeightT;          // 0 = root, 1 = tip
layout(location = 3) in float fragSide;             // -1 = left edge, 1 = right edge
layout(location = 4) flat in float fragBladeRandom; // Per-blade random value for color variance

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

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightDirection; // xyz = Light Direction
    vec4 lightColor;     // rgb = Light Color
} light;

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

// Blade color palette
const vec3 ROOT_COLOR_A = vec3(0.035, 0.12, 0.02);
const vec3 ROOT_COLOR_B = vec3(0.06, 0.18, 0.03);
const vec3 TIP_COLOR_A = vec3(0.28, 0.55, 0.10);
const vec3 TIP_COLOR_B = vec3(0.55, 0.62, 0.15);

void main()
{
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    // Per-blade base colors
    vec3 rootColor = mix(ROOT_COLOR_A, ROOT_COLOR_B, fragBladeRandom);
    vec3 tipColor = mix(TIP_COLOR_A, TIP_COLOR_B, fract(fragBladeRandom * 3.17));

    // Non-linear gradient so the deep green lingers a bit before the tip color takes over
    float heightMask = pow(clamp(fragHeightT, 0.0, 1.0), 0.7);
    vec3 baseColor = mix(rootColor, tipColor, heightMask);
    baseColor *= meshMatInfo.diffuseColor.rgb; // Apply tint with diffuse color that coming from material

    // Normalized fundamental vectors
    vec3 normalWorldSpace = normalize(fragNormal);
    vec3 normalizedLightDir = normalize(-light.lightDirection.xyz);
    vec3 normalizedView = normalize(pc.cameraPosition.xyz - fragWorldPos);

    // Flip normal to the view direction
    if (dot(normalWorldSpace, normalizedView) < 0.0)
    {
        normalWorldSpace = -normalWorldSpace;
    }

    // Ambient calculation
    vec3 ambient = meshMatInfo.ambientStrength * baseColor;

    // Diffuse calculation
    // Grass is thin enough that some light wraps around the blade instead of giving a hard terminator
    // like on solid geometry
    const float wrap = 0.5;
    float diff = max((dot(normalWorldSpace, normalizedLightDir) + wrap) / (1.0 + wrap), 0.0);
    float centerFalloff = 1.0 - abs(fragSide) * 0.35; // Fake rounding of the blade cross-section
    vec3 diffuse = diff * light.lightColor.rgb * baseColor * centerFalloff;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalWorldSpace, halfDir), 0.0), meshMatInfo.shininess);
    vec3 specular = meshMatInfo.specularStrength * spec * light.lightColor.rgb * meshMatInfo.specularColor.rgb;

    vec3 finalColor = ambient + diffuse + specular;
    outColor = vec4(finalColor, 1.0);
}
