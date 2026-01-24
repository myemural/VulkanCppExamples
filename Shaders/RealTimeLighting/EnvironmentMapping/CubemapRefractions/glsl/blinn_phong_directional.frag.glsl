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

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in mat3 fragTBN;

struct MeshData {
    mat4 model;
    mat4 normalMatrix;
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float opacity;
    float reflectivity;
    int diffuseMap;
    int specularMap;
    int normalMap;
    int emissiveMap;
    int shininessMap;
    int opacityMap;
    int aoMap;
    int heightMap;
};

layout(std430, binding = 0) readonly buffer MeshDataBuffer {
    MeshData meshes[];
};

layout(std140, set = 0, binding = 1) uniform LightUBO
{
    vec4 lightDirection; // xyz = Light Direction
    vec4 lightColor;     // rgb = Light Color
} light;

layout(set = 0, binding = 3) uniform samplerCube uEnvironmentMap;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    mat4 reflectionViewProj;
    vec4 cameraPosition;
    uint objectId;
} pc;

void main()
{
    // Get mesh info
    const MeshData meshInfo = meshes[pc.objectId];

    vec3 diffuseColor = meshInfo.diffuseColor.rgb;

    // Normal map calculation
    vec3 normalWorldSpace = normalize(fragTBN[2]);

    // Normalizing view direction (camera position)
    vec3 normalizedView = normalize(fragPos - pc.cameraPosition.xyz);

    // Refraction calculation
    const float ior = 1.5; // For glass material
    const float eta = 1.0 / ior; // For air

    // Do not consider exit pass refraction
    vec3 refractDir = refract(normalizedView, normalWorldSpace, eta);
    refractDir.z *= -1.0;
    vec3 refractionColor = texture(uEnvironmentMap, refractDir).rgb;

    // Ambient calculation
    vec3 ambient = meshInfo.ambientStrength * diffuseColor;

    // Final color
    const float refractionFactor = 0.8;
    vec3 finalColor = mix(ambient, refractionColor, refractionFactor);
    outColor = vec4(finalColor, meshInfo.opacity);
}