#version 450
#extension GL_EXT_mesh_shader : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 32) in;

const uint kTaskGroupSize = 32U;
const uint kLodLevelCount = 4U;

// Meshlet offset and count per LOD level
struct LodRange {
    uint meshletOffset;
    uint meshletCount;
};

layout(std430, set = 0, binding = 4) readonly buffer LodInfoBuffer { LodRange lodRanges[kLodLevelCount]; };

layout(push_constant) uniform LodPushConstants {
    mat4 modelMatrix;
    mat4 projViewMatrix;
    vec4 cameraPos;    // xyz = Camera Position
    vec4 lodDistances; // x = LOD0, y = LOD1, z = LOD2, w = LOD3
} pc;

// Payload that contains meshlet indices (for sending mesh shader)
struct TaskPayload {
    uint meshletIndices[kTaskGroupSize];
};
taskPayloadSharedEXT TaskPayload payload;

shared uint sVisibleCount;

uint selectLod(float distanceToCamera)
{
    if (distanceToCamera < pc.lodDistances.x) {
        return 0U;
    }
    if (distanceToCamera < pc.lodDistances.y) {
        return 1U;
    }
    if (distanceToCamera < pc.lodDistances.z) {
        return 2U;
    }
    return 3U;
}

void main()
{
    if (gl_LocalInvocationIndex == 0U) {
        sVisibleCount = 0U;
    }
    barrier();

    // Every thread in this task group belongs to the same draw, so the LOD decision is identical for all of them
    const vec3 instanceWorldPos = vec3(pc.modelMatrix[3]);
    const float distanceToCamera = length(instanceWorldPos - pc.cameraPos.xyz);
    const uint lod = selectLod(distanceToCamera);
    const LodRange range = lodRanges[lod];

    const uint localMeshletIndex = gl_GlobalInvocationID.x;
    const bool activeIndex = localMeshletIndex < range.meshletCount;

    if (activeIndex) {
        const uint slot = atomicAdd(sVisibleCount, 1U);
        payload.meshletIndices[slot] = range.meshletOffset + localMeshletIndex;
    }
    barrier();

    EmitMeshTasksEXT(sVisibleCount, 1, 1);
}
