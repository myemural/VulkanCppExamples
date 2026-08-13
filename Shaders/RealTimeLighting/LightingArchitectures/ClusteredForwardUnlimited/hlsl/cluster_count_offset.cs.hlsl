// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define TILE_SIZE 16
#define Z_SLICE_COUNT 16

struct PointLightData
{
    float4 lightPositionIntensity;    // xyz = View-space Position, w = Light Intensity
    float4 lightColorRadius;          // rgb = Light Color, a = Radius
};
[[vk::binding(3, 0)]] StructuredBuffer<PointLightData> lights;

struct ClusterHeader
{
    uint offset;
    uint count;
};
[[vk::binding(4, 0)]] RWStructuredBuffer<ClusterHeader> clusterHeaders;

[[vk::binding(5, 0)]] RWStructuredBuffer<uint> globalLightIndices;

[[vk::binding(6, 0)]] RWStructuredBuffer<uint> globalLightRefCount;

struct PushConstants
{
    float4x4 proj;
    float4 screenSizeAndPlanes; // x = Width, y = Height, z = Near Plane, w = Far Plane
    uint lightCount;
};
[[vk::push_constant]] PushConstants pc;

groupshared uint localCount;

void computeClusterBounds(uint clusterIndex, out float3 clusterMin, out float3 clusterMax, out float3 clusterCenter, out float clusterRadius)
{
    uint z = clusterIndex % Z_SLICE_COUNT;
    uint tmp = clusterIndex / Z_SLICE_COUNT;

    uint tilesX = (uint)(pc.screenSizeAndPlanes.x + TILE_SIZE - 1) / TILE_SIZE;
    uint x = tmp % tilesX;
    uint y = tmp / tilesX;

    float2 tileMin = float2(x, y) * TILE_SIZE;
    float2 tileMax = tileMin + TILE_SIZE;

    float2 ndcMin = float2(-1.0 + 2.0 * tileMin.x / pc.screenSizeAndPlanes.x, 1.0 - 2.0 * tileMax.y / pc.screenSizeAndPlanes.y);
    float2 ndcMax = float2(-1.0 + 2.0 * tileMax.x / pc.screenSizeAndPlanes.x, 1.0 - 2.0 * tileMin.y / pc.screenSizeAndPlanes.y);

    const float nearPlane = pc.screenSizeAndPlanes.z;
    const float farPlane = pc.screenSizeAndPlanes.w;
    float sliceNear = nearPlane * pow(farPlane / nearPlane, float(z) / float(Z_SLICE_COUNT));
    float sliceFar = nearPlane * pow(farPlane / nearPlane, float(z+1) / float(Z_SLICE_COUNT));

    // View-space Z is negative forward
    float zMinVS = -sliceFar;
    float zMaxVS = -sliceNear;

    float zScale = -zMaxVS;
    float2 projScale = float2(pc.proj[0][0], pc.proj[1][1]);

    float2 minXY = ndcMin * zScale / projScale;
    float2 maxXY = ndcMax * zScale / projScale;

    clusterMin = float3(minXY, zMinVS);
    clusterMax = float3(maxXY, zMaxVS);

    clusterCenter = (clusterMin + clusterMax) * 0.5;
    clusterRadius = length(clusterMax - clusterCenter);
}

bool sphereAabbTest(float3 center, float radius, float3 minB, float3 maxB)
{
    float3 clamped = clamp(center, minB, maxB);
    float3 delta = center - clamped;
    return dot(delta, delta) <= radius * radius;
}

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID,
    uint3 groupID         : SV_GroupID,
    uint3 groupThreadID   : SV_GroupThreadID,
    uint groupIndex       : SV_GroupIndex)
{
    uint clusterIndex = groupID.x;
    uint localThreadCount = TILE_SIZE * TILE_SIZE;

    if (groupIndex == 0) {
        localCount = 0;
    }

    float3 cMin, cMax, cCenter;
    float cRadius;
    computeClusterBounds(clusterIndex, cMin, cMax, cCenter, cRadius);

    GroupMemoryBarrierWithGroupSync();

    uint baseOffset = clusterHeaders[clusterIndex].offset;
    for (uint i = groupIndex; i < pc.lightCount; i += localThreadCount)
    {
        float3 center = lights[i].lightPositionIntensity.xyz;
        float radius = lights[i].lightColorRadius.a;

        // Distance culling (optional)
        float dist = length(center - cCenter);
        if (dist > radius + cRadius) {
            continue;
        }

        if (sphereAabbTest(center, radius, cMin, cMax))
        {
            uint localIdx;
            InterlockedAdd(localCount, 1, localIdx);
        }
    }

    GroupMemoryBarrierWithGroupSync();

    if(groupIndex == 0) {
        uint offset;
        InterlockedAdd(globalLightRefCount[0], localCount, offset);

        clusterHeaders[clusterIndex].offset = offset;
        clusterHeaders[clusterIndex].count  = localCount;
    }
}
