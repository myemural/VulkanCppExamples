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

layout(constant_id = 0) const uint TEXTURE_COUNT = 1;

struct MeshData {
    mat4 model;
    mat4 normalMatrix;
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float opacity;
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

layout(set = 0, binding = 2) uniform sampler2D uCombinedSamplers[TEXTURE_COUNT];

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDirTS, sampler2D heightMap, float heightScale)
{
    // Change layer count to the view angle
    const float minLayers = 8.0;
    const float maxLayers = 256.0;
    float numLayers = mix(maxLayers, minLayers, abs(viewDirTS.z));

    float layerStep = 1.0 / numLayers;

    // Grazing-angle fade
    float ndotv = clamp(viewDirTS.z, 0.0, 1.0);
    float parallaxFade = smoothstep(0.0, 0.2, ndotv);

    float viewDirZ = max(viewDirTS.z, 1e-4);
    vec2 P = (viewDirTS.xy / viewDirZ) * heightScale * parallaxFade;
    vec2 deltaTexCoords = P / numLayers;

    // Ray starts at top of heightfield
    float rayHeight = 1.0;

    vec2 currentTexCoords = texCoords;
    float currentHeight = texture(heightMap, currentTexCoords).r;

    // Previous step data (for interpolation)
    vec2 prevTexCoords = currentTexCoords;
    float prevHeight = currentHeight;
    float prevRayHeight = rayHeight;

    // Ray marching
    while (rayHeight > currentHeight)
    {
        prevTexCoords = currentTexCoords;
        prevHeight = currentHeight;
        prevRayHeight = rayHeight;

        currentTexCoords -= deltaTexCoords;
        rayHeight -= layerStep;
        currentHeight = texture(heightMap, currentTexCoords).r;
    }

    // Linear interpolation between last two steps
    float after  = currentHeight - rayHeight;
    float before = prevHeight - prevRayHeight;

    float weight = before / (before - after);
    vec2 finalTexCoords = mix(currentTexCoords, prevTexCoords, weight);

    return finalTexCoords;
}

void main()
{
    // Get mesh info
    const MeshData meshInfo = meshes[pc.objectId];

    // Parallax mapping calculation
    vec2 uv = fragUv;
    if (meshInfo.heightMap != -1) {
        vec3 viewDirWorldSpace = normalize(pc.cameraPosition.xyz - fragPos);
        vec3 viewDirTangentSpace = normalize(transpose(fragTBN) * viewDirWorldSpace);

        // Parallax strength
        const float parallaxScale = 0.05;

        // UV offset
        uv = ParallaxOcclusionMapping(fragUv, viewDirTangentSpace, uCombinedSamplers[meshInfo.heightMap], parallaxScale);
    }

    vec3 diffuseColor = meshInfo.diffuseColor.rgb;
    if (meshInfo.diffuseMap != -1) {
        vec4 diffuseTextureColor = texture(uCombinedSamplers[meshInfo.diffuseMap], uv);
        diffuseColor = diffuseTextureColor.rgb;
    }

    // Normal map calculation
    vec3 normalWorldSpace;
    if (meshInfo.normalMap != -1) {
        // Normal map sample (tangent space)
        vec3 normalTangent = texture(uCombinedSamplers[meshInfo.normalMap], uv).rgb;

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
    vec3 ambient = meshInfo.ambientStrength * diffuseColor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalWorldSpace, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * diffuseColor;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalWorldSpace, halfDir), 0.0), meshInfo.shininess);
    vec3 specular = meshInfo.specularStrength * spec * light.lightColor.rgb * meshInfo.specularColor.rgb;

    // Final color
    vec3 finalColor = ambient + diffuse + specular;
    outColor = vec4(finalColor, meshInfo.opacity);
}