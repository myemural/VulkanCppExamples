// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

// Light types
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT       1
#define LIGHT_TYPE_SPOT        2

struct PSInput
{
    [[vk::location(0)]] float3 fragPos : TEXCOORD0;
    [[vk::location(1)]] float3 fragNormal : TEXCOORD1;
};

[[vk::constant_id(0)]] const uint LIGHT_COUNT = 1;

struct MeshData
{
    float4x4 model;
    float4x4 normalMatrix;
    float4 diffuseColor;
    float4 specularColor;
    float  ambientStrength;
    float  shininess;
    float  specularStrength;
    float  opacity;
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

struct LightData
{
    float4 lightPosition;    // xyz = Light Position (unused for directional lights)
    float4 lightDirection;   // xyz = Light Direction (normalized, unused for point lights)
    float4 lightColor;       // rgb = Light Color, a = Light Intensity

    float4 lightTypeParams;  // x = Light Type (0: Directional, 1: Point: 2: Spot)
    float4 pointLightParams; // x = Constant Factor, y = Linear Factor, z = Quadratic Factor
    float4 spotlightParams;  // x = cos(innerCutoffAngle), y = cos(outerCutoffAngle)
};
[[vk::binding(1, 0)]] StructuredBuffer<LightData> lights : register(t1);

float3 calculateLight(LightData light, float3 normalizedNormal, float3 fragmentPosition, float3 normalizedView)
{
    // Get mesh info
    MeshData meshInfo = meshes[pc.objectId];

    // Get light type
    int type = int(light.lightTypeParams.x);

    // Calculate normalized light direction
    float3 normalizedLightDir;
    if (type == LIGHT_TYPE_DIRECTIONAL)
    {
        normalizedLightDir = normalize(-light.lightDirection.xyz);
    }
    else
    {
        normalizedLightDir = normalize(light.lightPosition.xyz - fragmentPosition);
    }

    float attenuation = 1.0;

    // Calculate attenuation contribuiton for spotlight
    if (type == LIGHT_TYPE_POINT)
    {
        float dist = length(light.lightPosition.xyz - fragmentPosition);
        attenuation = 1.0 / (light.pointLightParams.x + light.pointLightParams.y * dist + light.pointLightParams.z * dist * dist);
    }

    // Calculate spotlight calculation (with no attenuation)
    if (type == LIGHT_TYPE_SPOT)
    {
        float3 spotDir = normalize(-light.lightDirection.xyz);
        float theta = dot(normalizedLightDir, spotDir);
        float epsilon = light.spotlightParams.x - light.spotlightParams.y;

        float spotFactor = clamp((theta - light.spotlightParams.y) / epsilon, 0.0, 1.0);

        attenuation *= spotFactor;
    }

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalizedNormal, normalizedLightDir), 0.0);
    float3 diffuse = diff * light.lightColor.rgb * meshInfo.diffuseColor.rgb;

    // Specular calculation
    float3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalizedNormal, halfDir), 0.0), meshInfo.shininess);
    float3 specular = meshInfo.specularStrength * spec * light.lightColor.rgb * meshInfo.specularColor.rgb;

    // Final color
    float3 finalColor = attenuation * light.lightColor.a * (diffuse + specular);
    return finalColor;
}

float4 main(PSInput input) : SV_Target
{
    // Normalizing normal
    float3 normalizedNormal = normalize(input.fragNormal);

    // Normalizing view direction (camera position)
    float3 normalizedView = normalize(pc.cameraPosition.xyz - input.fragPos);

    float3 resultColor = 0.0;
    for (int i = 0; i < LIGHT_COUNT; i++) {
        resultColor += calculateLight(lights[i], normalizedNormal, input.fragPos, normalizedView);
    }

    return float4(resultColor, meshes[pc.objectId].opacity);
}