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

layout(location = 0) in vec3 fragPosView;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in mat3 fragTbnView;

// Constants
#define TILE_SIZE 16
#define MAX_LIGHTS_PER_CLUSTER 32
#define Z_SLICE_COUNT 16

struct MeshMaterialData
{
    vec4 diffuseColor;
    int diffuseMap;
    int normalMap;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(set = 0, binding = 2) uniform sampler2D uCombinedSamplers[];

struct PointLightData
{
    vec4 lightPositionIntensity;    // xyz = View-space Position, w = LightIntensity
    vec4 lightColorRadius;          // rgb = Light Color, a = Radius
};

layout(std430, set = 0, binding = 3) readonly buffer PointLightBuffer {
    PointLightData lights[];
};

struct ClusterLightList
{
    uint count;
    uint indices[MAX_LIGHTS_PER_CLUSTER];
};

layout(std430, set = 0, binding = 4) readonly buffer ClusterLightListBuffer
{
    ClusterLightList clusterLights[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    uint tilesX;
    uint objectId;
    float nearPlane;
    float farPlane;
} pc;

vec3 calculateLight(PointLightData light, vec3 albedo, vec3 normalView, vec3 fragPosView, vec3 viewDir)
{
    // Constants
    const float specularStrength = 0.7;
    const float shininess = 128.0;

    float radius = light.lightColorRadius.a;
    float intensity = light.lightPositionIntensity.w;

    vec3 lightVec = light.lightPositionIntensity.xyz - fragPosView;
    float dist = length(lightVec);

    // Normalize light direction
    vec3 lightDirView = lightVec / dist;

    // Smooth attenuation calculation
    float attenuation = intensity / (dist * dist + 1.0);
    float smoothFactor = 1.0 - (dist / radius);
    smoothFactor = clamp(smoothFactor, 0.0, 1.0);
    smoothFactor *= smoothFactor;
    attenuation *= smoothFactor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalView, lightDirView), 0.0);
    vec3 diffuse = diff * light.lightColorRadius.rgb * albedo;

    // Specular calculation
    vec3 halfDir = normalize(lightDirView + viewDir);
    float NdotH = max(dot(normalView, halfDir), 0.0);
    float specNorm = (shininess + 2.0) / 16.0; // Normalization factor for Blinn-Phong
    float spec = specNorm * pow(NdotH, shininess);
    vec3 specular = spec * light.lightColorRadius.rgb;

    // Final color (attenuation has less effect on specular)
    vec3 finalColor = (diffuse + specular) * attenuation;
    return finalColor;
}

void main()
{
    // Get mesh info
    const MeshMaterialData meshMaterial = meshMaterials[pc.objectId];

    vec4 diffuseColor = meshMaterial.diffuseColor.rgba;
    if (meshMaterial.diffuseMap != -1) {
        diffuseColor = texture(uCombinedSamplers[nonuniformEXT(meshMaterial.diffuseMap)], fragUv);
    }

    // Normal map calculation
    vec3 normalViewSpace;
    if (meshMaterial.normalMap != -1) {
        // Normal map sample (tangent space)
        vec3 normalTangent = texture(uCombinedSamplers[nonuniformEXT(meshMaterial.normalMap)], fragUv).rgb;

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

    // Cluster lookup
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    ivec2 tile  = pixel / TILE_SIZE;

    float viewZ = -fragPosView.z;   // View-space forward
    float logZ = (log(viewZ) - log(pc.nearPlane)) / (log(pc.farPlane) - log(pc.nearPlane));
    uint zSlice = uint(clamp(logZ * Z_SLICE_COUNT, 0.0, float(Z_SLICE_COUNT - 1)));
    uint clusterIndex = zSlice + Z_SLICE_COUNT * (tile.y * pc.tilesX + tile.x);
    ClusterLightList clusterList = clusterLights[clusterIndex];

    // Lighting
    vec3 viewDir = normalize(-fragPosView);

    const float ambientStrength = 0.02;
    vec3 result = ambientStrength * diffuseColor.rgb;

    for (uint i = 0; i < clusterList.count; i++)
    {
        uint lightIndex = clusterList.indices[i];
        result += calculateLight(lights[lightIndex], diffuseColor.rgb, normalViewSpace, fragPosView, viewDir);
    }

    outColor = vec4(result, 1.0);
}