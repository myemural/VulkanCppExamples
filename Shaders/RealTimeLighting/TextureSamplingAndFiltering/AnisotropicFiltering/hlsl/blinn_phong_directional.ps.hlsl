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

struct MeshMaterialData
{
    float4 diffuseColor;
    float4 specularColor;
    float  ambientStrength;
    float  shininess;
    float  specularStrength;
    int diffuseMap;
};
[[vk::binding(1, 0)]] StructuredBuffer<MeshMaterialData> meshMaterials;

struct LightUBO
{
    float4 lightDirection; // xyz = Light Direction
    float4 lightColor;     // rgb = Light Color
};

[[vk::binding(2, 0)]] cbuffer Light
{
    LightUBO light;
};

[[vk::binding(3, 0)]] SamplerState uSamplers[];
[[vk::binding(3, 0)]] Texture2D uImages[];

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    float4 cameraPosition;
    uint objectId;
};
[[vk::push_constant]] MeshPushConstants pc;

float4 main(PSInput input) : SV_Target
{
    // Get mesh info
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];

    float3 diffuseColor = meshInfo.diffuseColor.rgb;
    if (meshInfo.diffuseMap != -1) {
        /// NOTE: Scale texture to see anisotropic filtering effect well
        float4 diffuseTextureColor = uImages[meshInfo.diffuseMap].Sample(uSamplers[meshInfo.diffuseMap], input.fragUv * 4.0);
        diffuseColor = diffuseTextureColor.rgb;
    }

    // Normalizing normal
    float3 normalizedNormal = normalize(input.fragNormal);

    // Normalizing light direction
    float3 normalizedLightDir = normalize(-light.lightDirection.xyz);

    // Normalizing view direction (camera position)
    float3 normalizedView = normalize(pc.cameraPosition.xyz - input.fragPos);

    // Ambient calculation
    float3 ambient = meshInfo.ambientStrength * diffuseColor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalizedNormal, normalizedLightDir), 0.0);
    float3 diffuse = diff * light.lightColor.rgb * diffuseColor;

    // Specular calculation
    float3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalizedNormal, halfDir), 0.0), meshInfo.shininess);
    float3 specular = meshInfo.specularStrength * spec * light.lightColor.rgb * meshInfo.specularColor.rgb;

    // Final color
    float3 finalColor = ambient + diffuse + specular;
    return float4(finalColor, 1.0);
}