// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define NULL_NODE 0xFFFFFFFFu
#define MAX_FRAGMENTS 8U

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
    int diffuseMap;
    int normalMap;
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

struct OitNode
{
    uint colorPacked;
    float depth;
    uint next;
};
[[vk::binding(4, 0)]] RWStructuredBuffer<OitNode> nodes;

[[vk::binding(5, 0)]] RWStructuredBuffer<uint> heads;

[[vk::binding(6, 0)]] RWStructuredBuffer<uint> nodeCount;

struct MeshPushConstants
{
    float4x4 view;
    float4x4 proj;
    float4 cameraPosition;
    uint objectId;
    uint screenWidth;
    uint screenHeight;
};
[[vk::push_constant]] MeshPushConstants pc;

uint packUnorm4x8(float4 value)
{
    value = saturate(value);

    uint4 v = uint4(round(value * 255.0));

    return (v.x & 0xFF) |
           ((v.y & 0xFF) << 8) |
           ((v.z & 0xFF) << 16) |
           ((v.w & 0xFF) << 24);
}

void main(PSInput input)
{
    // Get mesh info
    const MeshMaterialData meshInfo = meshMaterials[pc.objectId];

    float3 diffuseColor = meshInfo.diffuseColor.rgb;
    if (meshInfo.diffuseMap != -1) {
        uint diffuseMapIndex = NonUniformResourceIndex(meshInfo.diffuseMap);
        float4 diffuseTextureColor = uImages[diffuseMapIndex].Sample(uSamplers[diffuseMapIndex], input.fragUv);
        diffuseColor = diffuseTextureColor.rgb;
    }

    // Normal map calculation
    float3 normalWorldSpace;
    if (meshInfo.normalMap != -1) {
        // Normal map sample (tangent space)
        uint normalMapIndex = NonUniformResourceIndex(meshInfo.normalMap);
        float3 normalTangent = uImages[normalMapIndex].Sample(uSamplers[normalMapIndex], input.fragUv).rgb;

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
    float4 color = float4(finalColor, meshInfo.opacity);

    // Discard full transparent fragments for optimization
    if (color.a < 0.01) {
        discard;
    }

    // Get new node index
    uint nodeIndex;
    InterlockedAdd(nodeCount[0], 1U, nodeIndex);

    // If buffer full, discard the fragment
    if (nodeIndex >= MAX_FRAGMENTS * pc.screenWidth * pc.screenHeight) {
        return;
    }

    // Calculate pixel index
    int2 coords = int2(input.position.xy);
    uint pixelIdx = uint(coords.y) * pc.screenWidth + uint(coords.x);

    // Pack the color and fill the node
    nodes[nodeIndex].colorPacked = packUnorm4x8(color);
    nodes[nodeIndex].depth = input.position.z;
    uint prevHead;
    InterlockedExchange(heads[pixelIdx], nodeIndex, prevHead);
    nodes[nodeIndex].next = prevHead;
}
