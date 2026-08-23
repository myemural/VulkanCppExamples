// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PSInput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float3 fragPos : POSITION;
    [[vk::location(1)]] float2 fragUv : TEXCOORD0;
    [[vk::location(2)]] float3x3 fragTBN : NORMAL;
};

struct MeshMaterialData
{
    float4 diffuseColor;
    float4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float opacity;
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

[[vk::binding(3, 0)]] SamplerState prevDepthSampler;
[[vk::binding(3, 0)]] Texture2D prevDepthTex;
[[vk::binding(4, 0)]] SamplerState opaqueDepthSampler;
[[vk::binding(4, 0)]] Texture2D opaqueDepthTex;

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    float4 cameraPosition;
    uint objectId;
    uint layerIndex;
};
[[vk::push_constant]] MeshPushConstants pc;

struct PSOutput
{
    [[vk::location(0)]] float4 outColor: SV_Target0;
    [[vk::location(1)]] float outDepth : SV_Target1;
};

PSOutput main(PSInput input)
{
    PSOutput output = (PSOutput)0;

    int2 texelCoord = int2(input.position.xy);
    float prevDepth = prevDepthTex.Load(int3(texelCoord, 0)).r;
    float opaqueDepth = opaqueDepthTex.Load(int3(texelCoord, 0)).r;

    // For first layer
    if (input.position.z >= opaqueDepth) {
        discard;
    }

    // For other layers
    if (pc.layerIndex > 0 && input.position.z <= prevDepth) {
        discard;
    }

    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    float3 diffuseColor = meshMatInfo.diffuseColor.rgb;

    // Normal map calculation
    float3 normalWorldSpace = normalize(input.fragTBN[2]);

    // Normalizing light direction
    float3 normalizedLightDir = normalize(-light.lightDirection.xyz);

    // Normalizing view direction (camera position)
    float3 normalizedView = normalize(pc.cameraPosition.xyz - input.fragPos);

    // Ambient calculation
    float3 ambient = meshMatInfo.ambientStrength * diffuseColor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalWorldSpace, normalizedLightDir), 0.0);
    float3 diffuse = diff * light.lightColor.rgb * diffuseColor;

    // Specular calculation
    float3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalWorldSpace, halfDir), 0.0), meshMatInfo.shininess);
    float3 specular = meshMatInfo.specularStrength * spec * light.lightColor.rgb * meshMatInfo.specularColor.rgb;

    // Final color
    float3 finalColor = ambient + diffuse + specular;
    output.outColor = float4(finalColor, meshMatInfo.opacity);
    output.outDepth = input.position.z;
    return output;
}
