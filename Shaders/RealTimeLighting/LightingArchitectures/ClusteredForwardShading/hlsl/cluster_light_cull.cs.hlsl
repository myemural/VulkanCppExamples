// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define TILE_SIZE 16
#define MAX_LIGHTS_PER_CLUSTER 32
#define Z_SLICE_COUNT 16

struct PointLightData
{
    float4 lightPositionIntensity;    // xyz = View-space Position, w = Light Intensity
    float4 lightColorRadius;          // rgb = Light Color, a = Radius
};
[[vk::binding(3, 0)]] StructuredBuffer<PointLightData> lights;

struct ClusterLightList
{
    uint count;
    uint indices[MAX_LIGHTS_PER_CLUSTER];
};
[[vk::binding(4, 0)]] RWStructuredBuffer<ClusterLightList> clusterLights;

struct PushConstants
{
    float4x4 proj;
    float4 screenSize;
    uint lightCount;
    float nearPlane;
    float farPlane;
};
[[vk::push_constant]] PushConstants pc;

bool sphereAabbTest(float3 center, float radius, float3 minB, float3 maxB)
{
    float3 clamped = clamp(center, minB, maxB);
    float3 delta = center - clamped;
    return dot(delta, delta) <= radius * radius;
}

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID,
    uint3 groupID         : SV_GroupID,
    uint3 groupThreadID   : SV_GroupThreadID,
    uint groupIndex       : SV_GroupIndex)
{
    uint3 clusterCoord = groupID;

    uint tilesX = (uint)(pc.screenSize.x + TILE_SIZE - 1) / TILE_SIZE;
    uint clusterIndex = clusterCoord.z + Z_SLICE_COUNT * (clusterCoord.y * tilesX + clusterCoord.x);

    // Compute cluster bounds in view space
    float2 invScreen = 1.0 / pc.screenSize.xy;

    float2 tileMin = (float2)clusterCoord.xy * TILE_SIZE;
    float2 tileMax = tileMin + TILE_SIZE;

    float xMinNDC = -1.0 + 2.0 * tileMin.x * invScreen.x;
    float xMaxNDC = -1.0 + 2.0 * tileMax.x * invScreen.x;

    float yMinNDC =  1.0 - 2.0 * tileMax.y * invScreen.y;
    float yMaxNDC =  1.0 - 2.0 * tileMin.y * invScreen.y;

    float zSlice = float(clusterCoord.z);

    float sliceNear = pc.nearPlane * pow(pc.farPlane / pc.nearPlane, zSlice / float(Z_SLICE_COUNT));
    float sliceFar = pc.nearPlane * pow(pc.farPlane / pc.nearPlane, (zSlice + 1.0) / float(Z_SLICE_COUNT));

    // View-space Z is negative forward
    float zMinVS = -sliceFar;
    float zMaxVS = -sliceNear;

    float P00 = pc.proj[0][0];
    float P11 = pc.proj[1][1];

    // Convert NDC to view space extents
    float xMinVS = xMinNDC * (-zMaxVS) / P00;
    float xMaxVS = xMaxNDC * (-zMaxVS) / P00;

    float yMinVS = yMinNDC * (-zMaxVS) / P11;
    float yMaxVS = yMaxNDC * (-zMaxVS) / P11;

    float3 clusterMin = float3(xMinVS, yMinVS, zMinVS);
    float3 clusterMax = float3(xMaxVS, yMaxVS, zMaxVS);

    // Light culling
    uint count = 0;
    for (uint i = 0; i < pc.lightCount; ++i) {
        float3 center = lights[i].lightPositionIntensity.xyz;
        float radius = lights[i].lightColorRadius.a;

        if (sphereAabbTest(center, radius, clusterMin, clusterMax)) {
            if (count < MAX_LIGHTS_PER_CLUSTER) {
                clusterLights[clusterIndex].indices[count] = i;
                count++;
            } else {
                break;
            }
        }
    }

    clusterLights[clusterIndex].count = count;
}
