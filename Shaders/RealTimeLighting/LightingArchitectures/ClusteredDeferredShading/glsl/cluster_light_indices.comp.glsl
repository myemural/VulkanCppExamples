#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 64) in;

#define TILE_SIZE 16
#define Z_SLICE_COUNT 16

struct PointLightData
{
    vec4 lightPosition;    // xyz = View-space Position
    vec4 lightColorRadius; // rgb = Light Color, a = Radius
};

layout(std430, set = 0, binding = 0) readonly buffer PointLightBuffer
{
    PointLightData lights[];
};

struct ClusterHeader
{
    uint offset;
    uint count;
};

layout(std430, set = 0, binding = 1) readonly buffer ClusterHeaderBuffer
{
    ClusterHeader clusterHeaders[];
};

layout(std430, set=0, binding = 2) buffer GlobalLightIndexBuffer
{
    uint globalLightIndices[];
};

layout(push_constant) uniform PushConstants
{
    mat4 proj;
    vec4 screenSizeAndPlanes; // x = Width, y = Height, z = Near Plane, w = Far Plane
    uint lightCount;
} pc;

shared uint writeCounter;

void computeClusterBounds(uint clusterIndex, out vec3 clusterMin, out vec3 clusterMax, out vec3 clusterCenter, out float clusterRadius)
{
    uint z = clusterIndex % Z_SLICE_COUNT;
    uint tmp = clusterIndex / Z_SLICE_COUNT;

    uint tilesX = uint((pc.screenSizeAndPlanes.x + TILE_SIZE - 1) / TILE_SIZE);
    uint x = tmp % tilesX;
    uint y = tmp / tilesX;

    vec2 tileMin = vec2(x, y) * TILE_SIZE;
    vec2 tileMax = tileMin + TILE_SIZE;

    vec2 ndcMin = vec2(-1.0 + 2.0 * tileMin.x / pc.screenSizeAndPlanes.x, 1.0 - 2.0 * tileMax.y / pc.screenSizeAndPlanes.y);
    vec2 ndcMax = vec2(-1.0 + 2.0 * tileMax.x / pc.screenSizeAndPlanes.x, 1.0 - 2.0 * tileMin.y / pc.screenSizeAndPlanes.y);

    const float nearPlane = pc.screenSizeAndPlanes.z;
    const float farPlane = pc.screenSizeAndPlanes.w;
    float sliceNear = nearPlane * pow(farPlane / nearPlane, float(z) / float(Z_SLICE_COUNT));
    float sliceFar = nearPlane * pow(farPlane / nearPlane, float(z+1) / float(Z_SLICE_COUNT));

    // View-space Z is negative forward
    float zMinVS = -sliceFar;
    float zMaxVS = -sliceNear;

    float zScale = -zMaxVS;
    vec2 projScale = vec2(pc.proj[0][0], pc.proj[1][1]);

    vec2 minXY = ndcMin * zScale / projScale;
    vec2 maxXY = ndcMax * zScale / projScale;

    clusterMin = vec3(minXY, zMinVS);
    clusterMax = vec3(maxXY, zMaxVS);

    clusterCenter = (clusterMin + clusterMax) * 0.5;
    clusterRadius = length(clusterMax - clusterCenter);
}

bool sphereAabbTest(vec3 center, float radius, vec3 minB, vec3 maxB)
{
    vec3 clamped = clamp(center, minB, maxB);
    vec3 delta = center - clamped;
    return dot(delta, delta) <= radius * radius;
}

void main()
{
    uint clusterIndex = gl_WorkGroupID.x;

    if (gl_LocalInvocationIndex == 0) {
        writeCounter = 0;
    }

    vec3 cMin, cMax, cCenter;
    float cRadius;
    computeClusterBounds(clusterIndex, cMin, cMax, cCenter, cRadius);

    barrier();

    uint baseOffset = clusterHeaders[clusterIndex].offset;
    for (uint i = gl_LocalInvocationIndex; i < pc.lightCount; i += gl_WorkGroupSize.x)
    {
        vec3 center = lights[i].lightPosition.xyz;
        float radius = lights[i].lightColorRadius.a;

        // Distance culling (optional)
        float dist = length(center - cCenter);
        if (dist > radius + cRadius) {
            continue;
        }

        if (sphereAabbTest(center, radius, cMin, cMax))
        {
            uint localIdx = atomicAdd(writeCounter, 1);
            globalLightIndices[baseOffset + localIdx] = i;
        }
    }
}