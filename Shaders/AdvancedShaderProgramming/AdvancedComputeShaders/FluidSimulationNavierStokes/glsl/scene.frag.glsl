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
layout(location = 2) in vec3 fragNormal;

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

layout(std140, set = 0, binding = 2) uniform SceneUBO
{
    vec4 lightDirection; // xyz = Light Direction
    vec4 lightColor;     // xyz = Light Color
    vec4 volumeMin;      // xyz = Smoke Volume Min Corner, w = Smoke Absorption
    vec4 volumeMax;      // xyz = Smoke Volume Max Corner
} scene;

layout(set = 0, binding = 3) uniform sampler3D smokeVolume; // w channel holds the density

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

const int kShadowSteps = 12;

// Beer-Lambert transmittence from a surface point towards the light
float smokeShadow(vec3 worldPos, vec3 toLight)
{
    vec3 inverseDir = 1.0 / (toLight + 1e-8);
    vec3 t0 = (scene.volumeMin.xyz - worldPos) * inverseDir;
    vec3 t1 = (scene.volumeMax.xyz - worldPos) * inverseDir;
    vec3 tSmall = min(t0, t1);
    vec3 tLarge = max(t0, t1);

    float start = max(max(max(tSmall.x, tSmall.y), tSmall.z), 0.0);
    float end = min(min(tLarge.x, tLarge.y), tLarge.z);

    if (end <= start) {
        return 1.0;
    }

    float stepSize = (end - start) / float(kShadowSteps);
    float opticalDepth = 0.0;
    vec3 volumeSize = scene.volumeMax.xyz - scene.volumeMin.xyz;

    for (int i = 0; i < kShadowSteps; ++i)
    {
        vec3 samplePos = worldPos + toLight * (start + (float(i) + 0.5) * stepSize);
        opticalDepth += textureLod(smokeVolume, (samplePos - scene.volumeMin.xyz) / volumeSize, 0.0).w;
    }

    return exp(-opticalDepth * stepSize * scene.volumeMin.w);
}

void main()
{
    const MeshMaterialData material = meshMaterials[pc.objectId];

    vec3 normal = normalize(fragNormal);
    vec3 toLight = normalize(-scene.lightDirection.xyz);
    vec3 toView = normalize(pc.cameraPosition.xyz - fragPos);

    float shadow = smokeShadow(fragPos + normal * 1e-3, toLight);

    // Ambient calculation
    vec3 ambient = material.ambientStrength * material.diffuseColor.rgb;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normal, toLight), 0.0);
    vec3 diffuse = diff * shadow * scene.lightColor.rgb * material.diffuseColor.rgb;

    // Specular calculation
    vec3 halfDir = normalize(toLight + toView);
    float spec = pow(max(dot(normal, halfDir), 0.0), material.shininess);
    vec3 specular = material.specularStrength * spec * shadow * scene.lightColor.rgb * material.specularColor.rgb;

    // Final color
    vec3 finalColor = ambient + diffuse + specular;
    outColor = vec4(finalColor, material.opacity);
}
