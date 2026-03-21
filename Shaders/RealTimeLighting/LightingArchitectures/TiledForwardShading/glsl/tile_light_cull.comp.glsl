#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

#define TILE_SIZE 16
#define MAX_LIGHTS_PER_TILE 64
#define FRUSTUM_PLANE_COUNT 6

struct PointLightData
{
    vec4 lightPositionIntensity;    // xyz = View-space Position, w = Light Intensity
    vec4 lightColorRadius;          // rgb = Light Color, a = Radius
};

layout(std430, set = 0, binding = 3) readonly buffer PointLightBuffer
{
    PointLightData lights[];
};

struct TileLightList
{
    uint count;
    uint indices[MAX_LIGHTS_PER_TILE];
};

layout(std430, set = 0, binding = 4) buffer TileLightListBuffer
{
    TileLightList tileLights[];
};

layout(set = 0, binding = 5) uniform sampler2D depthTex;

layout(push_constant) uniform PushConstants
{
    mat4 proj;
    vec4 screenSize;
    uint lightCount;
} pc;

// Shared variables
shared vec4 planes[FRUSTUM_PLANE_COUNT];
shared uint lightCountPerTile;
shared float minDepth;
shared float maxDepth;

bool sphereFrustumTest(vec3 center, float radius)
{
    for (int i = 0; i < FRUSTUM_PLANE_COUNT; i++) {
        float dist = dot(planes[i].xyz, center) + planes[i].w;
        if (dist < -radius) {
            return false;
        }
    }
    return true;
}

void main()
{
    uvec2 tileCoord = gl_WorkGroupID.xy;
    uint localThreadIndex = gl_LocalInvocationIndex;
    uint localThreadCount = gl_WorkGroupSize.x * gl_WorkGroupSize.y;

    uint tilesX = uint((pc.screenSize.x + TILE_SIZE - 1) / TILE_SIZE);
    uint tilesY = uint((pc.screenSize.y + TILE_SIZE - 1) / TILE_SIZE);
    uint tileIndex = tileCoord.y * tilesX + tileCoord.x;

    // Tile setup and frustum plane calculations
    if (localThreadIndex == 0)
    {
        tileLights[tileIndex].count = 0;
        lightCountPerTile = 0;

        // Min/max depth calculation
        float minD = 1.0;
        float maxD = 0.0;

        vec2 invScreen = 1.0 / pc.screenSize.xy;
        vec2 tileBase  = vec2(tileCoord * TILE_SIZE);

        for (int y = 0; y < TILE_SIZE; ++y)
        {
            float fy = (tileBase.y + float(y)) * invScreen.y;

            for (int x = 0; x < TILE_SIZE; ++x)
            {
                float fx = (tileBase.x + float(x)) * invScreen.x;
                float d = texture(depthTex, vec2(fx, fy)).x;
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

        vec2 tileMin = tileBase;
        vec2 tileMax = tileMin + TILE_SIZE;

        float sx = 2.0 * invScreen.x;
        float sy = 2.0 * invScreen.y;

        float xMin = -1.0 + tileMin.x * sx;
        float xMax = -1.0 + tileMax.x * sx;

        float yMin =  1.0 - tileMax.y * sy;
        float yMax =  1.0 - tileMin.y * sy;

        planes[0] = vec4( P00, 0.0,  xMin, 0.0);
        planes[1] = vec4(-P00, 0.0, -xMax, 0.0);
        planes[2] = vec4(0.0,  P11,  yMin, 0.0);
        planes[3] = vec4(0.0, -P11, -yMax, 0.0);

        float A = pc.proj[2][2];
        float B = pc.proj[3][2];

        float zNearVS = B / (minDepth - A);
        float zFarVS  = B / (maxDepth - A);

        planes[4] = vec4(0.0, 0.0, -1.0, -zNearVS);
        planes[5] = vec4(0.0, 0.0,  1.0,  zFarVS);
    }

    barrier();

    // Light culling
    for (uint i = localThreadIndex; i < pc.lightCount && lightCountPerTile < MAX_LIGHTS_PER_TILE; i += localThreadCount)
    {
        vec3 center = lights[i].lightPositionIntensity.xyz;
        float radius = lights[i].lightColorRadius.a;

        if (sphereFrustumTest(center, radius)) {
            uint index = atomicAdd(lightCountPerTile, 1);
            if (index < MAX_LIGHTS_PER_TILE) {
                tileLights[tileIndex].indices[index] = i;
            }
        }
    }

    barrier();

    // Write total light counts per tile
    if (localThreadIndex == 0) {
        tileLights[tileIndex].count = min(lightCountPerTile, MAX_LIGHTS_PER_TILE);
    }
}