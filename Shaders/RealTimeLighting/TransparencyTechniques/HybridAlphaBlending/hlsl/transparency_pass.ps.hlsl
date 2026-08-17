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

[[vk::constant_id(0)]] const uint LIGHT_COUNT = 0;

struct PointLightData
{
    float4 lightPosition;    // xyz = Light Position (View-Space)
    float4 lightColor;       // rgb = Light Color
    float4 pointLightParams; // x = Constant Factor, y = Linear Factor, z = Quadratic Factor
};
[[vk::binding(3, 0)]] StructuredBuffer<PointLightData> lights;

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    uint objectId;
};
[[vk::push_constant]] MeshPushConstants pc;

float3 calculateLight(PointLightData light, float3 albedo, float3 normalView, float3 fragPosView, float3 viewDir)
{
    // Constants
    const float specularStrength = 1.0;
    const float shininess = 64.0;

    float dist = length(light.lightPosition.xyz - fragPosView);
    float attenuation = 1.0 / (light.pointLightParams.x + light.pointLightParams.y * dist + light.pointLightParams.z * dist * dist);;

    float3 lightDirView = normalize(light.lightPosition.xyz - fragPosView);

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalView, lightDirView), 0.0);
    float3 diffuse = diff * light.lightColor.rgb * albedo;

    // Specular calculation
    float3 halfDir = normalize(lightDirView + viewDir);
    float spec = pow(max(dot(normalView, halfDir), 0.0), shininess);
    float3 specular = specularStrength * spec * light.lightColor.rgb;

    // Final color (attenuation has less effect on specular)
    float3 finalColor = (diffuse * attenuation) + (specular * sqrt(attenuation));
    return finalColor;
}

float4 main(PSInput input) : SV_Target
{
    // Constants
    const float ambientStrength = 0.02;

    // Get mesh info
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];

    float3 albedo = meshInfo.diffuseColor.rgb;
    if (meshInfo.diffuseMap != -1) {
        uint diffuseMapIndex = NonUniformResourceIndex(meshInfo.diffuseMap);
        float4 diffuseTextureColor = uImages[diffuseMapIndex].Sample(uSamplers[diffuseMapIndex], input.fragUv);
        albedo = diffuseTextureColor.rgb;
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

    // Lighting vectors in view space
    float3 viewDir = normalize(-input.fragPosView);

    // Ambient calculation
    float3 ambient = ambientStrength * albedo;

    float3 resultColor = 0.0;
    for (uint i = 0; i < LIGHT_COUNT; i++) {
        resultColor += calculateLight(lights[i], albedo, normalViewSpace, input.fragPosView, viewDir);
    }
    resultColor += ambient;

    return float4(resultColor, meshInfo.opacity);
}
