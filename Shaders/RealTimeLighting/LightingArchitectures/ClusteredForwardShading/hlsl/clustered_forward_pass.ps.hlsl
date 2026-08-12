// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

// Constants
#define TILE_SIZE 16
#define MAX_LIGHTS_PER_CLUSTER 32
#define Z_SLICE_COUNT 16

struct PSInput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float3 fragPosView : POSITION;
    [[vk::location(1)]] float2 fragUv : TEXCOORD0;
    [[vk::location(2)]] float3x3 fragTbnView : NORMAL;
};

struct MeshMaterialData
{
    float4 diffuseColor;
    int diffuseMap;
    int normalMap;
};
[[vk::binding(1, 0)]] StructuredBuffer<MeshMaterialData> meshMaterials;

[[vk::binding(2, 0)]] SamplerState uSamplers[];
[[vk::binding(2, 0)]] Texture2D uImages[];

struct PointLightData
{
    float4 lightPositionIntensity;    // xyz = View-space Position, w = Light Intensity
    float4 lightColorRadius;          // rgb = Light Color, a = Radius
};
[[vk::binding(3, 0)]] StructuredBuffer<PointLightData> lights;

struct ClusterLightList
{
    uint count;
    uint indices[MAX_LIGHTS_PER_CLUSTER];
};
[[vk::binding(4, 0)]] StructuredBuffer<ClusterLightList> clusterLights;

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    uint tilesX;
    uint objectId;
    float nearPlane;
    float farPlane;
};
[[vk::push_constant]] MeshPushConstants pc;

float3 calculateLight(PointLightData light, float3 albedo, float3 normalView, float3 fragPosView, float3 viewDir)
{
    // Constants
    const float specularStrength = 0.7;
    const float shininess = 128.0;

    float radius = light.lightColorRadius.a;
    float intensity = light.lightPositionIntensity.w;

    float3 lightVec = light.lightPositionIntensity.xyz - fragPosView;
    float dist = length(lightVec);

    // Normalize light direction
    float3 lightDirView = lightVec / dist;

    // Smooth attenuation calculation
    float attenuation = intensity / (dist * dist + 1.0);
    float smoothFactor = 1.0 - (dist / radius);
    smoothFactor = clamp(smoothFactor, 0.0, 1.0);
    smoothFactor *= smoothFactor;
    attenuation *= smoothFactor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalView, lightDirView), 0.0);
    float3 diffuse = diff * light.lightColorRadius.rgb * albedo;

    // Specular calculation
    float3 halfDir = normalize(lightDirView + viewDir);
    float NdotH = max(dot(normalView, halfDir), 0.0);
    float specNorm = (shininess + 2.0) / 16.0; // Normalization factor for Blinn-Phong
    float spec = specNorm * pow(NdotH, shininess);
    float3 specular = spec * light.lightColorRadius.rgb;

    // Final color (attenuation has less effect on specular)
    float3 finalColor = (diffuse + specular) * attenuation;
    return finalColor;
}

float4 main(PSInput input) : SV_Target
{
    // Get mesh info
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];

    float4 diffuseColor = meshInfo.diffuseColor;
    if (meshInfo.diffuseMap != -1) {
        uint diffuseMapIndex = NonUniformResourceIndex(meshInfo.diffuseMap);
        diffuseColor = uImages[diffuseMapIndex].Sample(uSamplers[diffuseMapIndex], input.fragUv);
    }

    // Normal map calculation
    float3 normalViewSpace;
    if (meshInfo.normalMap != -1) {
        // Normal map sample (tangent space)
        uint normalMapIndex = NonUniformResourceIndex(meshInfo.normalMap);
        float3 normalTangent = uImages[normalMapIndex].Sample(uSamplers[normalMapIndex], input.fragUv).rgb;

        // Transform from [0,1] to [-1,1] range
        normalTangent = normalize(normalTangent * 2.0 - 1.0);

        // Tangent space to view space
        normalViewSpace = normalize(mul(normalTangent, input.fragTbnView));
    }
    else
    {
        // Fallback: Geometric normal
        normalViewSpace = normalize(input.fragTbnView[2]);
    }

    // Tile lookup
    int2 pixel = int2(input.position.xy);
    int2 tile = pixel / TILE_SIZE;

    float viewZ = -input.fragPosView.z;   // View-space forward
    float logZ = (log(viewZ) - log(pc.nearPlane)) / (log(pc.farPlane) - log(pc.nearPlane));
    uint zSlice = uint(clamp(logZ * Z_SLICE_COUNT, 0.0, float(Z_SLICE_COUNT - 1)));
    uint clusterIndex = zSlice + Z_SLICE_COUNT * (tile.y * pc.tilesX + tile.x);
    ClusterLightList clusterList = clusterLights[clusterIndex];

    // Lighting
    float3 viewDir = normalize(-input.fragPosView);

    const float ambientStrength = 0.02;
    float3 result = ambientStrength * diffuseColor.rgb;

    for (uint i = 0; i < clusterList.count; i++)
    {
        uint lightIndex = clusterList.indices[i];
        result += calculateLight(lights[lightIndex], diffuseColor.rgb, normalViewSpace, input.fragPosView, viewDir);
    }

    return float4(result, 1.0);
}
