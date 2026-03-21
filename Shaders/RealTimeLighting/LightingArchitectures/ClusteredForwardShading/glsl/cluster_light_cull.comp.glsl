#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#define TILE_SIZE 16
#define MAX_LIGHTS_PER_CLUSTER 32
#define Z_SLICE_COUNT 16

struct PointLightData
{
    vec4 lightPositionIntensity;    // xyz = View-space Position, w = LightIntensity
    vec4 lightColorRadius;          // rgb = Light Color, a = Radius
};

layout(std430, set = 0, binding = 3) readonly buffer PointLightBuffer
{
    PointLightData lights[];
};

struct ClusterLightList
{
    uint count;
    uint indices[MAX_LIGHTS_PER_CLUSTER];
};

layout(std430, set = 0, binding = 4) buffer ClusterLightListBuffer
{
    ClusterLightList clusterLights[];
};

layout(push_constant) uniform PushConstants
{
    mat4 proj;
    vec4 screenSize;
    uint lightCount;
    float nearPlane;
    float farPlane;
} pc;

bool sphereAabbTest(vec3 center, float radius, vec3 minB, vec3 maxB)
{
    vec3 clamped = clamp(center, minB, maxB);
    vec3 delta = center - clamped;
    return dot(delta, delta) <= radius * radius;
}

void main()
{
    uvec3 clusterCoord = gl_WorkGroupID.xyz;

    uint tilesX = uint((pc.screenSize.x + TILE_SIZE - 1) / TILE_SIZE);
    uint clusterIndex = clusterCoord.z + Z_SLICE_COUNT * (clusterCoord.y * tilesX + clusterCoord.x);

    // Compute cluster bounds in view space
    vec2 invScreen = 1.0 / pc.screenSize.xy;

    vec2 tileMin = vec2(clusterCoord.xy) * TILE_SIZE;
    vec2 tileMax = tileMin + TILE_SIZE;

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

    vec3 clusterMin = vec3(xMinVS, yMinVS, zMinVS);
    vec3 clusterMax = vec3(xMaxVS, yMaxVS, zMaxVS);

    // Light culling
    uint count = 0;
    for (uint i = 0; i < pc.lightCount; ++i) {
        vec3 center = lights[i].lightPositionIntensity.xyz;
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