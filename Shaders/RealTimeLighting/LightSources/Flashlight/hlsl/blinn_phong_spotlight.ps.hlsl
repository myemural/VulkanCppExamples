// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PSInput
{
    [[vk::location(0)]] float3 fragPos : TEXCOORD0;
    [[vk::location(1)]] float3 fragNormal : TEXCOORD1;
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
    float4 lightPosition;    // xyz = Light Position
    float4 lightDirection;   // xyz = Light Direction (normalized)
    float4 lightColor;       // rgb = Light Color
    float4 spotlightParams;  // x = cos(cutoffAngle)
};

[[vk::binding(2, 0)]] cbuffer Light
{
    LightUBO light;
};

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    float4 cameraPosition;
    uint objectId;
};
[[vk::push_constant]] MeshPushConstants pc;

float3 calculateLight(float3 normalizedNormal, float3 fragmentPosition)
{
    // Get mesh info
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];

    // Normalizing light direction
    float3 normalizedLightDir = normalize(light.lightPosition.xyz - fragmentPosition);

    // Normalizing view direction (camera position)
    float3 normalizedView = normalize(pc.cameraPosition.xyz - fragmentPosition);

    // Ambient calculation
    float3 ambient = meshInfo.ambientStrength * meshInfo.diffuseColor.rgb;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalizedNormal, normalizedLightDir), 0.0);
    float3 diffuse = diff * light.lightColor.rgb * meshInfo.diffuseColor.rgb;

    // Specular calculation
    float3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalizedNormal, halfDir), 0.0), meshInfo.shininess);
    float3 specular = meshInfo.specularStrength * spec * light.lightColor.rgb * meshInfo.specularColor.rgb;

    // Spotlight contribution calculation (hard-cutoff)
    float3 spotDir = normalize(-light.lightDirection.xyz);
    float theta = dot(normalizedLightDir, spotDir);

    float innerCutoff = light.spotlightParams.x;
    float outerCutoff = light.spotlightParams.y;
    float spotFactor = smoothstep(outerCutoff, innerCutoff, theta);

    // Final color
    float3 finalColor = ambient + spotFactor * (diffuse + specular);;
    return finalColor;
}

float4 main(PSInput input) : SV_Target
{
    // Normalizing normal
    float3 normalizedNormal = normalize(input.fragNormal);

    float3 resultColor = 0.0;
    resultColor += calculateLight(normalizedNormal, input.fragPos);
    return float4(resultColor, 1.0);
}