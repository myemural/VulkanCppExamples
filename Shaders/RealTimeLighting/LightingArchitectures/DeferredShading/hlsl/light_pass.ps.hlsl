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
};

[[vk::binding(0, 0)]] SamplerState gPositionSampler;
[[vk::binding(0, 0)]] Texture2D gPositionTex;
[[vk::binding(1, 0)]] SamplerState gAlbedoSampler;
[[vk::binding(1, 0)]] Texture2D gAlbedoTex;
[[vk::binding(2, 0)]] SamplerState gNormalSampler;
[[vk::binding(2, 0)]] Texture2D gNormalTex;

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
    // Debug mode for output
    // 0: Off
    // 1: Albedo Output
    // 2: Normalized View-Space Position Output
    // 3: Normalized View-Space Normal Output
    uint debugMode;
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

    // Sampling G-Buffer
    uint width;
    uint height;
    gPositionTex.GetDimensions(width, height);
    float2 uv = input.position.xy / float2(width, height);

    float3 fragPosView = gPositionTex.Sample(gPositionSampler, uv).xyz;
    float4 albedoSample = gAlbedoTex.Sample(gAlbedoSampler, uv);
    float3 albedo = albedoSample.rgb;
    float opacity = albedoSample.a;

    if (opacity <= 0.001) {
        return float4(albedo, 1.0);
    }

    float3 normalView = gNormalTex.Sample(gNormalSampler, uv).rgb;
    normalView = normalize(normalView);

    // Lighting vectors in view space
    float3 viewDir = normalize(-fragPosView);

    // Ambient calculation
    float3 ambient = ambientStrength * albedo;

    float3 resultColor = 0.0;
    for (uint i = 0; i < LIGHT_COUNT; i++) {
        resultColor += calculateLight(lights[i], albedo, normalView, fragPosView, viewDir);
    }
    resultColor += ambient;

    if (pc.debugMode == 0) {
        return float4(resultColor, opacity);
    } else if (pc.debugMode == 1) {
        return float4(albedo, opacity);
    } else if (pc.debugMode == 2) {
        return float4(abs(fragPosView) / 20.0, opacity);
    } else if (pc.debugMode == 3) {
        return float4(normalView * 0.5 + 0.5, opacity);
    }

    return float4(resultColor, opacity);
}
