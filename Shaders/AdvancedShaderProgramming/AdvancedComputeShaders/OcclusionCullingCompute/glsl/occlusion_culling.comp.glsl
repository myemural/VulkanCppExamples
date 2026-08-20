#version 450
#extension GL_KHR_shader_subgroup_basic : require
#extension GL_KHR_shader_subgroup_ballot : require
#extension GL_KHR_shader_subgroup_vote : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 64) in;

struct ObjectBounds
{
    vec4 center; // xyz = World-space AABB center
    vec4 extent; // xyz = World-space AABB half extent
};

layout(std430, binding = 0) readonly buffer ObjectBoundsBuffer {
    ObjectBounds objectBounds[];
};

layout(std430, binding = 1) buffer CullingResultsBuffer {
    uint frustumCulledCount; // Frustum culled object count, index = 0
    uint occlusionCulledCount; // Occlusion culled object count, index = 1
    uint pad0_; // Padding, index = 2
    uint pad1_; // Padding, index = 3
    uint visibility[]; // Visibility value for each object in the scene, 0U: Not-visible, 1U: Visible, index = 4
} cullingResults;

// Full Hi-Z mip pyramid
layout(binding = 2) uniform sampler2D hizPyramid;

layout (push_constant) uniform PushConstants {
    mat4 viewProjMatrix;
    vec2 hizBaseSize;
    uint hizMipcount;
    uint objectCount;
    uint frustumCullingEnabled;
    uint occlusionCullingEnabled;
} pc;

// Gribb/Hartmann method: Frustum planes fall directly out of the combined view-proj matrix rows
void extractFrustumPlanes(out vec4 planes[6])
{
    const mat4 rows = transpose(pc.viewProjMatrix);

    planes[0] = rows[3] + rows[0]; // Left
    planes[1] = rows[3] - rows[0]; // Right
    planes[2] = rows[3] + rows[1]; // Bottom
    planes[3] = rows[3] - rows[1]; // Top
    planes[4] = rows[3] + rows[2]; // Near
    planes[5] = rows[3] - rows[2]; // Far
}

// Standard AABB-vs-plane test
bool isAabbInFrustum(vec3 center, vec3 extent)
{
    vec4 planes[6];
    extractFrustumPlanes(planes);

    for (int i = 0; i < 6; i++) {
        const float signedDistance = dot(planes[i].xyz, center) + planes[i].w;
        const float projectRadius = dot(abs(planes[i].xyz), extent);

        // Fully behind this plane, definitely outside the frustum
        if (signedDistance + projectRadius < 0.0) {
            return false;
        }
    }

    return true;
}

bool isAabbOccluded(vec3 center, vec3 extent)
{
    vec2 uvMin = vec2(1e30);
    vec2 uvMax = vec2(-1e30);
    float closestDepth = 1e30;

    // Project all 8 AABB corners to screen space to build a conservative screen-space bounding rect + nearest depth
    for (int i = 0; i < 8; ++i) {
        const vec3 corner = center + extent * vec3((i & 1) != 0 ? 1.0 : -1.0,
                                                   (i & 2) != 0 ? 1.0 : -1.0,
                                                   (i & 4) != 0 ? 1.0 : -1.0);
        const vec4 clipPos = pc.viewProjMatrix * vec4(corner, 1.0);
        if (clipPos.w <= 0.0) {
            return false;
        }

        const vec3 ndcPos = clipPos.xyz / clipPos.w;

        // Vulkan's framebuffer origin is the top-left corner, the projection matrix already carrying Y-flip
        // So NDC maps straight onto texture coordinates
        const vec2 uv = ndcPos.xy * 0.5 + 0.5;

        uvMin = min(uvMin, uv);
        uvMax = max(uvMax, uv);
        closestDepth = min(closestDepth, ndcPos.z);
    }

    uvMin = clamp(uvMin, vec2(0.0), vec2(1.0));
    uvMax = clamp(uvMax, vec2(0.0), vec2(1.0));
    closestDepth = clamp(closestDepth, 0.0, 1.0);

    // Pick the coarsest mip whose texel footprint still covers the object's screen rect
    // So a single 2x2 tap gives the true max depth under the whole rect (not just part of it)
    const vec2 footprintPixels = (uvMax - uvMin) * pc.hizBaseSize;
    const float longestSide = max(footprintPixels.x, footprintPixels.y);
    const int mipLevel = clamp(int(ceil(log2(max(longestSide, 1.0)))), 0, int(pc.hizMipcount) - 1);

    const ivec2 mipSize = max(ivec2(pc.hizBaseSize) >> mipLevel, ivec2(1));
    const ivec2 texelMin = clamp(ivec2(uvMin * vec2(mipSize)), ivec2(0), mipSize - ivec2(1));
    const ivec2 texelMax = clamp(ivec2(uvMax * vec2(mipSize)), ivec2(0), mipSize - ivec2(1));

    // Sample the 4 corner texels of the rect at the chosen mip and take the max
    float farthestSceneDepth = texelFetch(hizPyramid, ivec2(texelMin.x, texelMin.y), mipLevel).r;
    farthestSceneDepth = max(farthestSceneDepth, texelFetch(hizPyramid, ivec2(texelMax.x, texelMin.y), mipLevel).r);
    farthestSceneDepth = max(farthestSceneDepth, texelFetch(hizPyramid, ivec2(texelMin.x, texelMax.y), mipLevel).r);
    farthestSceneDepth = max(farthestSceneDepth, texelFetch(hizPyramid, ivec2(texelMax.x, texelMax.y), mipLevel).r);

    // Object is occluded only if its nearest point is farther than the scene's farthest occluder depth in that region
    return closestDepth > farthestSceneDepth;
}

void main()
{
    const uint objectId = gl_GlobalInvocationID.x;
    const bool isInRange = objectId < pc.objectCount;

    // Clamp out-of-range lanes to a valid index (ID 0) purely to keep the buffer read in-bounds
    const uint safeId = isInRange ? objectId : 0U;
    const vec3 boundsCenter = objectBounds[safeId].center.xyz;
    const vec3 boundsExtent = objectBounds[safeId].extent.xyz;

    const bool isFrustumPassed = isInRange &&
                                 (pc.frustumCullingEnabled == 0U || isAabbInFrustum(boundsCenter, boundsExtent));

    bool isOccluded = false;
    // Skip the occlusion test entirely for this whole subgroup if every lane already failed frustum culling
    if (subgroupAny(isFrustumPassed)) {
        if (isFrustumPassed && pc.occlusionCullingEnabled != 0U) {
            isOccluded = isAabbOccluded(boundsCenter, boundsExtent);
        }
    }

    if (isInRange) {
        cullingResults.visibility[objectId] = (isFrustumPassed && !isOccluded) ? 1U : 0U;
    }

    // Reduce per-lane cull results across the subgroup
    // So only one atomicAdd per subgroup is needed instead of one per object
    const uint frustumCulledCount = subgroupBallotBitCount(subgroupBallot(isInRange && !isFrustumPassed));
    const uint occlusionCulledCount = subgroupBallotBitCount(subgroupBallot(isFrustumPassed && isOccluded));

    // Exactly one lane per subgroup performs the atomic update
    if (subgroupElect()) {
        if (frustumCulledCount > 0U) {
            atomicAdd(cullingResults.frustumCulledCount, frustumCulledCount);
        }

        if (occlusionCulledCount > 0U) {
            atomicAdd(cullingResults.occlusionCulledCount, occlusionCulledCount);
        }
    }
}
