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
layout(location = 1) out float outDepth;

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
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightDirection; // xyz = Light Direction
    vec4 lightColor;     // rgb = Light Color
} light;

layout(set = 0, binding = 3) uniform sampler2D prevDepthTex;
layout(set = 0, binding = 4) uniform sampler2D opaqueDepthTex;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    uint layerIndex;
} pc;

void main()
{
    ivec2 texelCoord = ivec2(gl_FragCoord.xy);
    float prevDepth = texelFetch(prevDepthTex, texelCoord, 0).r;
    float opaqueDepth = texelFetch(opaqueDepthTex, texelCoord, 0).r;

    // For first layer
    if (gl_FragCoord.z >= opaqueDepth) {
        discard;
    }

    // For other layers
    if (pc.layerIndex > 0 && gl_FragCoord.z <= prevDepth) {
        discard;
    }

    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    vec3 diffuseColor = meshMatInfo.diffuseColor.rgb;

    // Normal map calculation
    vec3 normalWorldSpace = normalize(fragTBN[2]);

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
    outColor = vec4(finalColor, meshMatInfo.opacity);
    outDepth = gl_FragCoord.z;
}