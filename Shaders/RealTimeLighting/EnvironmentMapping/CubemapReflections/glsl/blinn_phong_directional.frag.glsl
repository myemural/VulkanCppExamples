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

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(-light.lightDirection.xyz);

    // Normalizing view direction (camera position)
    vec3 normalizedView = normalize(pc.cameraPosition.xyz - fragPos);

    // Reflection vector
    vec3 reflectDir = reflect(-normalizedView, normalWorldSpace);
    reflectDir.z *= -1; // For flipping front and back

    // Sample cubemap
    vec3 reflectionColor = texture(uEnvironmentMap, reflectDir).rgb;

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

    // Adjust reflection
    const float shadowReflectionVisibility = 0.10;
    float shadowMask = clamp(diff + shadowReflectionVisibility, 0.0, 1.0); // Adjust reflection for the dark side of the object
    float reflectionFactor = meshInfo.specularStrength * shadowMask;
    finalColor = mix(finalColor, reflectionColor, reflectionFactor);

    outColor = vec4(finalColor, meshInfo.opacity);
}