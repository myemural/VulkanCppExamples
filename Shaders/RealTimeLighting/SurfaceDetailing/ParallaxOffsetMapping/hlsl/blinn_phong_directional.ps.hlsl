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
    int normalMap;
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

    // Parallax mapping calculation
    float2 uv = input.fragUv;
    if (meshInfo.heightMap != -1) {
        float3 viewDirWorldSpace = normalize(pc.cameraPosition.xyz - input.fragPos);
        float3 viewDirTangentSpace = mul(input.fragTBN, viewDirWorldSpace);

        // Height sample
        uint heightMapIndex = NonUniformResourceIndex(meshInfo.heightMap);
        float height = uImages[heightMapIndex].Sample(uSamplers[heightMapIndex], input.fragUv).r;

        // Parallax strength
        const float parallaxScale = 0.03;

        // Grazing-angle fade
        float ndotv = clamp(viewDirTangentSpace.z, 0.0, 1.0);
        float parallaxFade = smoothstep(0.0, 0.2, ndotv);

        // UV offset
        float viewDirTangentZ = max(viewDirTangentSpace.z, 1e-4);
        float2 parallaxOffset = (viewDirTangentSpace.xy * parallaxFade) / viewDirTangentZ * height * parallaxScale;
        uv -= parallaxOffset;
    }

    float3 diffuseColor = meshInfo.diffuseColor.rgb;
    if (meshInfo.diffuseMap != -1) {
        uint diffuseMapIndex = NonUniformResourceIndex(meshInfo.diffuseMap);
        float4 diffuseTextureColor = uImages[diffuseMapIndex].Sample(uSamplers[diffuseMapIndex], uv);
        diffuseColor = diffuseTextureColor.rgb;
    }

    // Normal map calculation
    float3 normalWorldSpace;
    if (meshInfo.normalMap != -1) {
        // Normal map sample (tangent space)
        uint normalMapIndex = NonUniformResourceIndex(meshInfo.normalMap);
        float3 normalTangent = uImages[normalMapIndex].Sample(uSamplers[normalMapIndex], uv).rgb;

        // Transform from [0,1] to [-1,1] range
        normalTangent = normalize(normalTangent * 2.0 - 1.0);

        // Tangent space to world space
        normalWorldSpace = normalize(mul(normalTangent, input.fragTBN));
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