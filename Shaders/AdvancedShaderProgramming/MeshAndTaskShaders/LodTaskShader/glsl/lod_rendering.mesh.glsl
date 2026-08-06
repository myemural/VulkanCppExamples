#version 450
#extension GL_EXT_mesh_shader : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

// Matches with the default limits (maxVertices = 64, maxTriangles = 124)
layout(local_size_x = 64) in;
layout(triangles, max_vertices = 64, max_primitives = 124) out;

layout(location = 0) out vec2 outUv[];
layout(location = 1) out vec3 outMeshletColor[];

layout(push_constant) uniform MeshPushConstants {
    mat4 modelMatrix;
    mat4 projViewMatrix;
    vec4 cameraPos;    // xyz = Camera Position
    vec4 lodDistances; // x = LOD0, y = LOD1, z = LOD2, w = LOD3
} pc;

struct MeshletDescriptor {
    uint vertexOffset;
    uint triangleOffset;
    uint vertexCount;
    uint triangleCount;
};

// Contains full vertex pool for the primitive, laid out as [pos.x, pos.y, pos.z, uv.x, uv.y]
layout(std430, set = 0, binding = 0) readonly buffer VertexBuffer { float vertexData[]; };

// Meshlet to vertex pool index remap
layout(std430, set = 0, binding = 1) readonly buffer MeshletVertexIndices { uint meshletVertexIndices[]; };

// Per-meshlet local triangle indices, expanded from uint8 to uint32 on the CPU side
layout(std430, set = 0, binding = 2) readonly buffer MeshletTriangleIndices { uint meshletTriangleIndices[]; };

// One MeshletDescriptor per meshlet
layout(std430, set = 0, binding = 3) readonly buffer MeshletDescriptors { MeshletDescriptor meshlets[]; };

const uint kElementCountPerVertex = 5U; // vec3 position + vec2 uv, tightly packed
const uint kTaskGroupSize = 32U;

struct TaskPayload {
    uint meshletIndices[kTaskGroupSize];
};
taskPayloadSharedEXT TaskPayload payload;

// Cheap integer hash to stable pseudo-random RGB per meshlet index (Debug-only)
vec3 MeshletDebugColor(uint meshletIndex)
{
    uint h = meshletIndex * 2654435761u;
    h ^= h >> 13u;
    h *= 0x85ebca6bu;
    h ^= h >> 16u;

    return vec3(float(h & 0xFFu), float((h >> 8u) & 0xFFu), float((h >> 16u) & 0xFFu)) / 255.0;
}

void main()
{
    const uint meshletIndex = payload.meshletIndices[gl_WorkGroupID.x];
    const MeshletDescriptor meshlet = meshlets[meshletIndex];

    // Every invocation in the workgroup must agree on this call before any output writes
    SetMeshOutputsEXT(meshlet.vertexCount, meshlet.triangleCount);

    const uint threadId = gl_LocalInvocationIndex;
    const vec3 debugColor = MeshletDebugColor(meshletIndex);

    // Vertices: local_size_x (64) == max_vertices, so one invocation per vertex
    if (threadId < meshlet.vertexCount) {
        const uint srcVertexIndex = meshletVertexIndices[meshlet.vertexOffset + threadId];
        const uint base = srcVertexIndex * kElementCountPerVertex;

        const vec3 position = vec3(vertexData[base + 0U], vertexData[base + 1U], vertexData[base + 2U]);
        const vec2 uv = vec2(vertexData[base + 3U], vertexData[base + 4U]);

        gl_MeshVerticesEXT[threadId].gl_Position = pc.projViewMatrix * pc.modelMatrix * vec4(position, 1.0);
        outUv[threadId] = uv;
        outMeshletColor[threadId] = debugColor;
    }

    // Triangles: Up to 124 primitives, so loop in strides of the workgroup size
    for (uint triIndex = threadId; triIndex < meshlet.triangleCount; triIndex += 64U) {
        const uint packedOffset = meshlet.triangleOffset + triIndex * 3U;
        const uint i0 = meshletTriangleIndices[packedOffset + 0U];
        const uint i1 = meshletTriangleIndices[packedOffset + 1U];
        const uint i2 = meshletTriangleIndices[packedOffset + 2U];

        gl_PrimitiveTriangleIndicesEXT[triIndex] = uvec3(i0, i1, i2);
    }
}
