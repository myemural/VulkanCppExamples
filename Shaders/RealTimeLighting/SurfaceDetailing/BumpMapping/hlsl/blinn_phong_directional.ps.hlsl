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
    [[vk::location(2)]] float3x3 fragTBN : NORMAL;
};

struct MeshMaterialData
{
    float4 diffuseColor;
    float4 specularColor;
    float  ambientStrength;
    float  shininess;
    float  specularStrength;
    int diffuseMap;
    int heightMap;
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
        uint diffuseMapIndex = NonUniformResourceIndex(meshInfo.diffuseMap);
        float4 diffuseTextureColor = uImages[diffuseMapIndex].Sample(uSamplers[diffuseMapIndex], input.fragUv);
        diffuseColor = diffuseTextureColor.rgb;
    }

    // Bump map calculation
    float3 normalWorldSpace;
    if (meshInfo.heightMap != -1) {
        // Texel size (guaranteed non-zero gradient)
        uint heightMapIndex = NonUniformResourceIndex(meshInfo.heightMap);
        uint textureWidth, textureHeight;
        uImages[heightMapIndex].GetDimensions(textureWidth, textureHeight);
        float2 texel = 1.0 / float2(textureWidth, textureHeight);
        float hC = uImages[heightMapIndex].Sample(uSamplers[heightMapIndex], input.fragUv).r;
        float hX = uImages[heightMapIndex].Sample(uSamplers[heightMapIndex], input.fragUv + float2(texel.x, 0.0)).r;
        float hY = uImages[heightMapIndex].Sample(uSamplers[heightMapIndex], input.fragUv + float2(0.0, texel.y)).r;

        // We are using a large scale to ensure the bump map effect is clearly visible
        float scale = 8.0;
        float dU = (hX - hC) * scale;
        float dV = (hY - hC) * scale;

        // Tangent-space bump normal
        float3 bumpTangentSpace = normalize(float3(-dU, -dV, 1.0));

        // To world space
        normalWorldSpace = normalize(mul(bumpTangentSpace, input.fragTBN));
    }
    else
    {
        // Fallback: Geometric normal
        normalWorldSpace = normalize(input.fragTBN[2]);
    }

    // Normalizing light direction
    float3 normalizedLightDir = normalize(-light.lightDirection.xyz);

    // Normalizing view direction (camera position)
    float3 normalizedView = normalize(pc.cameraPosition.xyz - input.fragPos);

    // Ambient calculation
    float3 ambient = meshInfo.ambientStrength * diffuseColor;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalWorldSpace, normalizedLightDir), 0.0);
    float3 diffuse = diff * light.lightColor.rgb * diffuseColor;

    // Specular calculation
    float3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalWorldSpace, halfDir), 0.0), meshInfo.shininess);
    float3 specular = meshInfo.specularStrength * spec * light.lightColor.rgb * meshInfo.specularColor.rgb;

    // Final color
    float3 finalColor = ambient + diffuse + specular;
    return float4(finalColor, 1.0);
}