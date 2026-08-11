// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define TILE_SIZE 16
#define MAX_LIGHTS_PER_TILE 64
#define FRUSTUM_PLANE_COUNT 6

struct PointLightData
{
    float4 lightPositionIntensity;    // xyz = View-space Position, w = Light Intensity
    float4 lightColorRadius;          // rgb = Light Color, a = Radius
};
[[vk::binding(3, 0)]] StructuredBuffer<PointLightData> lights;

struct TileLightList
{
    uint count;
    uint indices[MAX_LIGHTS_PER_TILE];
};
[[vk::binding(4, 0)]] RWStructuredBuffer<TileLightList> tileLights;

[[vk::binding(5, 0)]] SamplerState depthTexSampler;
[[vk::binding(5, 0)]] Texture2D depthTex;

struct PushConstants
{
    float4x4 proj;
    float4 screenSize;
    uint lightCount;
};
[[vk::push_constant]] PushConstants pc;

// Shared variables
groupshared float4 planes[FRUSTUM_PLANE_COUNT];
groupshared uint lightCountPerTile;
groupshared float minDepth;
groupshared float maxDepth;

bool sphereFrustumTest(float3 center, float radius)
{
    for (uint i = 0; i < FRUSTUM_PLANE_COUNT; ++i) {
        float dist = dot(planes[i].xyz, center) + planes[i].w;

        if (dist < -radius) {
            return false;
        }
    }

    return true;
}

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID,
    uint3 groupID         : SV_GroupID,
    uint3 groupThreadID   : SV_GroupThreadID,
    uint groupIndex       : SV_GroupIndex)
{
    uint2 tileCoord = groupID.xy;
    uint localThreadIndex = groupIndex;
    uint localThreadCount = TILE_SIZE * TILE_SIZE;

    uint tilesX = (uint)(pc.screenSize.x + TILE_SIZE - 1) / TILE_SIZE;
    uint tilesY = (uint)(pc.screenSize.y + TILE_SIZE - 1) / TILE_SIZE;
    uint tileIndex = tileCoord.y * tilesX + tileCoord.x;

    // Tile setup and frustum plane calculations
    if (localThreadIndex == 0)
    {
        tileLights[tileIndex].count = 0;
        lightCountPerTile = 0;

        // Min/max depth calculation
        float minD = 1.0;
        float maxD = 0.0;

        float2 invScreen = 1.0 / pc.screenSize.xy;
        float2 tileBase = (float2)tileCoord * TILE_SIZE;

        for (uint y = 0; y < TILE_SIZE; ++y)
        {
            float fy = (tileBase.y + (float)y) * invScreen.y;

            for (uint x = 0; x < TILE_SIZE; ++x)
            {
                float fx = (tileBase.x + (float)x) * invScreen.x;
                float d = depthTex.SampleLevel(depthTexSampler, float2(fx, fy), 0).x;
                minD = min(minD, d);
                maxD = max(maxD, d);
            }
        }

        minDepth = minD;
        maxDepth = maxD;

        if (minDepth > maxDepth) {
            maxDepth = minDepth;
        }

        // Build frustum planes
        float P00 = pc.proj[0][0];
        float P11 = pc.proj[1][1];

        float2 tileMin = tileBase;
        float2 tileMax = tileMin + TILE_SIZE;

        float sx = 2.0 * invScreen.x;
        float sy = 2.0 * invScreen.y;

        float xMin = -1.0 + tileMin.x * sx;
        float xMax = -1.0 + tileMax.x * sx;

        float yMin = 1.0 - tileMax.y * sy;
        float yMax = 1.0 - tileMin.y * sy;

        planes[0] = float4( P00, 0.0,  xMin, 0.0);
        planes[1] = float4(-P00, 0.0, -xMax, 0.0);
        planes[2] = float4(0.0,  P11,  yMin, 0.0);
        planes[3] = float4(0.0, -P11, -yMax, 0.0);

        float A = pc.proj[2][2];
        float B = pc.proj[2][3];

        float zNearVS = B / (minDepth - A);
        float zFarVS  = B / (maxDepth - A);

        planes[4] = float4(0.0, 0.0, -1.0, -zNearVS);
        planes[5] = float4(0.0, 0.0,  1.0,  zFarVS);
    }

    GroupMemoryBarrierWithGroupSync();

    // Light culling
    for (uint i = localThreadIndex; i < pc.lightCount && lightCountPerTile < MAX_LIGHTS_PER_TILE; i += localThreadCount)
    {
        float3 center = lights[i].lightPositionIntensity.xyz;
        float radius  = lights[i].lightColorRadius.a;

        if (sphereFrustumTest(center, radius))
        {
            uint index;
            InterlockedAdd(lightCountPerTile, 1, index);
            if (index < MAX_LIGHTS_PER_TILE)
            {
                tileLights[tileIndex].indices[index] = i;
            }
        }
    }

    GroupMemoryBarrierWithGroupSync();

    // Write total light counts per tile
    if (localThreadIndex == 0)
    {
        tileLights[tileIndex].count = min(lightCountPerTile, (uint)MAX_LIGHTS_PER_TILE);
    }
}
