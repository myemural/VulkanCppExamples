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

[[vk::binding(3, 0)]] SamplerState uEnvironmentMapSampler;
[[vk::binding(3, 0)]] TextureCube uEnvironmentMapTexture;

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

    // Normal map calculation
    float3 normalWorldSpace = normalize(input.fragTBN[2]);

    // Normalizing light direction
    float3 normalizedLightDir = normalize(-light.lightDirection.xyz);

    // Normalizing view direction (camera position)
    float3 normalizedView = normalize(pc.cameraPosition.xyz - input.fragPos);

    // Reflection vector
    float3 reflectDir = reflect(-normalizedView, normalWorldSpace);
    reflectDir.z *= -1; // For flipping front and back

    // Sample cubemap
    float3 reflectionColor = uEnvironmentMapTexture.Sample(uEnvironmentMapSampler, reflectDir).rgb;

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

    // Adjust reflection
    const float shadowReflectionVisibility = 0.10;
    float shadowMask = clamp(diff + shadowReflectionVisibility, 0.0, 1.0); // Adjust reflection for the dark side of the object
    float reflectionFactor = meshInfo.specularStrength * shadowMask;
    finalColor = lerp(finalColor, reflectionColor, reflectionFactor);

    return float4(finalColor, 1.0);
}