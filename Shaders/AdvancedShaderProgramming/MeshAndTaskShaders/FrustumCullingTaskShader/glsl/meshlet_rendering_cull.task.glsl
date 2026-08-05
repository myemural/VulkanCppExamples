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

struct MeshletBounds {
    vec3 center;
    float radius;
    vec3 coneAxis;
    float coneCutoff;
};

// Bounding sphere per meshlet (object-space)
layout(std430, set = 0, binding = 4) readonly buffer MeshletBoundsBuffer { MeshletBounds meshletBounds[]; };

// 6 frustum planes of the culling camera (world-space)
layout(std430, set = 0, binding = 5) readonly buffer FrustumPlanesBuffer { vec4 frustumPlanes[6]; };

layout(push_constant) uniform MeshPushConstants {
    mat4 modelMatrix;
    mat4 projViewMatrix;
    uint meshletCount;
} pc;

const uint kTaskGroupSize = 32U;

// Payload that contains meshlet indices (for sending mesh shader)
struct TaskPayload {
    uint meshletIndices[kTaskGroupSize];
};
taskPayloadSharedEXT TaskPayload payload;

shared uint sVisibleCount;

// Checks given sphere is in frustum or not
bool isSphereInFrustum(vec3 centerWorld, float radius)
{
    for (int i = 0; i < 6; ++i) {
        if (dot(frustumPlanes[i].xyz, centerWorld) + frustumPlanes[i].w < -radius) {
            return false;
        }
    }
    return true;
}

void main()
{
    if (gl_LocalInvocationIndex == 0U) {
        sVisibleCount = 0U;
    }
    barrier();

    const uint meshletIndex = gl_GlobalInvocationID.x;
    bool visible = false;

    // Visibility check
    if (meshletIndex < pc.meshletCount) {
        const MeshletBounds bounds = meshletBounds[meshletIndex];
        const vec3 centerWorld = (pc.modelMatrix * vec4(bounds.center, 1.0)).xyz;
        visible = isSphereInFrustum(centerWorld, bounds.radius);
    }

    // If visible, put meshlet indices to the payload
    if (visible) {
        const uint slot = atomicAdd(sVisibleCount, 1U);
        payload.meshletIndices[slot] = meshletIndex;
    }
    barrier();

    // Emit only visible meshes
    EmitMeshTasksEXT(sVisibleCount, 1, 1);
}
