// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PSInput
{
    [[vk::location(0)]] float3 fragPos : POSITION;
    [[vk::location(1)]] float2 fragUv : TEXCOORD0;
    [[vk::location(2)]] float3 fragNormal : NORMAL;
};

struct MeshData
{
    float4x4 model;
    float4x4 normalMatrix;
    float4 diffuseColor;
    float4 specularColor;
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
[[vk::binding(0, 0)]] StructuredBuffer<MeshData> meshes : register(t0);

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    float4 cameraPosition;
    uint objectId;
};
[[vk::push_constant]] MeshPushConstants pc;

struct LightUBO
{
    float4 lightPosition;    // xyz = Light Position
    float4 lightColor;       // rgb = Light Color
    float4 pointLightParams; // x = Constant Factor, y = Linear Factor, z = Quadratic Factor
};

[[vk::binding(1, 0)]]
cbuffer Light : register(b1)
{
    LightUBO light;
};

[[vk::binding(2, 0)]] SamplerState uSamplers[];
[[vk::binding(2, 0)]] Texture2D uImages[];

float4 main(PSInput input) : SV_Target
{
    // Get mesh info
    MeshData meshInfo = meshes[pc.objectId];

    float3 diffuseColor = meshInfo.diffuseColor.rgb;
    if (meshInfo.diffuseMap != -1) {
        float4 diffuseTextureColor = uImages[meshInfo.diffuseMap].Sample(uSamplers[meshInfo.diffuseMap], input.fragUv);
        diffuseColor = diffuseTextureColor.rgb;
    }

    float3 specularColor = meshInfo.specularColor.rgb;
    if (meshInfo.specularMap != -1) {
        float4 specularTextureColor = uImages[meshInfo.specularMap].Sample(uSamplers[meshInfo.specularMap], input.fragUv);
        specularColor = specularTextureColor.rgb;
    }

    // Normalizing normal
    float3 normalizedNormal = normalize(input.fragNormal);

    // Normalizing light direction
    float3 normalizedLightDir = normalize(light.lightPosition.xyz - input.fragPos);

    // Normalizing view direction (camera position)
    float3 normalizedView = normalize(pc.cameraPosition.xyz - input.fragPos);

    // Ambient calculation
    float3 ambient = meshInfo.ambientStrength * diffuseColor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(input.fragNormal, normalizedLightDir), 0.0);
    float3 diffuse = diff * light.lightColor.rgb * diffuseColor;

    // Shininess calculation
    float shininess = meshInfo.shininess;
    float energyComp = 1.0; // Fake energy compensation for better visualization of shininess mapping
    if (meshInfo.shininessMap != -1) {
        float minShine = 8.0;
        float maxShine = 1024.0;

        float roughness = uImages[meshInfo.shininessMap].Sample(uSamplers[meshInfo.shininessMap], input.fragUv).r;
        float perceptualRoughness = roughness * roughness;
        shininess = lerp(maxShine, minShine, perceptualRoughness);
        energyComp = lerp(1.0, 0.1, perceptualRoughness);
    }

    // Specular calculation
    float3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = min(pow(max(dot(normalizedNormal, halfDir), 0.0), shininess), 1.0);
    float3 specular = meshInfo.specularStrength * energyComp * spec * light.lightColor.rgb * specularColor;

    // Attenuation calculation
    float distance = length(light.lightPosition.xyz - input.fragPos);
    float attenuation = 1.0 / (light.pointLightParams.x + light.pointLightParams.y * distance + light.pointLightParams.z * (distance * distance));

    // Apply attenuation to all components
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // Final color
    float3 finalColor = ambient + diffuse + specular;
    return float4(finalColor, meshInfo.opacity);
}