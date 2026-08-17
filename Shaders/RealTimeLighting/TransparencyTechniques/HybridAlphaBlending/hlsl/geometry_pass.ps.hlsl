// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PSInput
{
    [[vk::location(0)]] float3 fragPosView : POSITION;
    [[vk::location(1)]] float2 fragUv : TEXCOORD0;
    [[vk::location(2)]] float3x3 fragTbnView : NORMAL;
};

struct MeshMaterialData
{
    float4 diffuseColor;
    float opacity;
    int diffuseMap;
    int normalMap;
};
[[vk::binding(1, 0)]] StructuredBuffer<MeshMaterialData> meshMaterials;

[[vk::binding(2, 0)]] SamplerState uSamplers[];
[[vk::binding(2, 0)]] Texture2D uImages[];

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    uint objectId;
};
[[vk::push_constant]] MeshPushConstants pc;

struct PSOutput
{
    [[vk::location(0)]] float4 gPosition: SV_Target0;
    [[vk::location(1)]] float4 gAlbedo : SV_Target1;
    [[vk::location(2)]] float4 gNormal : SV_Target2;
};

PSOutput main(PSInput input)
{
    PSOutput output = (PSOutput)0;

    // Get mesh info
    const MeshMaterialData meshMaterial = meshMaterials[pc.objectId];

    output.gPosition = float4(input.fragPosView, 1.0);

    float4 albedo = meshMaterial.diffuseColor.rgba;
    if (meshMaterial.diffuseMap != -1) {
        uint diffuseMapIndex = NonUniformResourceIndex(meshMaterial.diffuseMap);
        albedo = uImages[diffuseMapIndex].Sample(uSamplers[diffuseMapIndex], input.fragUv);
    }

    output.gAlbedo = albedo;

    // Normal map calculation
    float3 normalViewSpace;
    if (meshMaterial.normalMap != -1) {
        // Normal map sample (tangent space)
        uint normalMapIndex = NonUniformResourceIndex(meshMaterial.normalMap);
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

    output.gNormal = float4(normalViewSpace, 1.0);

    return output;
}
