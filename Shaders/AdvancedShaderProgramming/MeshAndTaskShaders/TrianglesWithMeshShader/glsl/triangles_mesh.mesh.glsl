#version 450
#extension GL_EXT_mesh_shader : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

// Workgroup of 3 threads (one thread per triangle, since we draw at most 3 triangles).
// Dispatched with (1, 1, 1) workgroups, so this workgroup's 3 invocations are the only ones that ever run.
layout(local_size_x = 3) in;

// This workgroup emits a triangle mesh with at most 9 vertices and 3 primitives (triangles).
// Each thread below is responsible for producing exactly one triangle (3 vertices + 1 primitive).
layout(triangles, max_vertices = 9, max_primitives = 3) out;

// Per-vertex output color.
layout(location = 0) out vec3 outColor[];

layout(push_constant) uniform MeshPushConstants {
    uint triangleCount;
} pc;

const uint kVertexPerTriangle = 3;
const uint kMaxtriangleCount = 3;

// Local-space shape of a single triangle (in NDC-like coordinates), shared by every triangle instance.
const vec2 kTriangleShape[kVertexPerTriangle] = vec2[](
        vec2(0.0, -0.5),  // Top
        vec2(0.22, 0.5),  // Right-bottom
        vec2(-0.22, 0.5)  // Left-bottom
);

// Per-triangle 2D offset used to lay the (up to 3) triangles out side-by-side on screen.
const vec2 kTriangleOffsets[kMaxtriangleCount] = vec2[](
        vec2(-0.55, 0.0),
        vec2(0.0, 0.0),
        vec2(0.55, 0.0)
);

// Color palette assigned to each triangle's 3 vertices.
const vec3 kVertexColors[kVertexPerTriangle] = vec3[kVertexPerTriangle](
        vec3(1.0, 0.2, 0.2),
        vec3(0.2, 1.0, 0.2),
        vec3(0.2, 0.4, 1.0)
);

void main()
{
    // Clamp the requested triangle count into the valid [1, kMaxtriangleCount] range so out-of-range push constant
    // values can't produce zero output.
    const uint triangleCount = clamp(pc.triangleCount, 1U, kMaxtriangleCount);

    // Mesh shaders require an explicit declaration, per-workgroup, of how many vertices and primitives will actually
    // be written this dispatch. Must be called before writing to gl_MeshVerticesEXT / gl_PrimitiveTriangleIndicesEXT,
    // and must be uniform across the workgroup.
    SetMeshOutputsEXT(triangleCount * kVertexPerTriangle, triangleCount);

    // Each of the 3 invocations in this workgroup owns one triangle.
    // gl_LocalInvocationIndex ranges [0, 2] given the local_size above.
    const uint triangleIndex = gl_LocalInvocationIndex;

    // Threads beyond the requested triangle count have nothing to do.
    if (triangleIndex >= triangleCount) {
        return;
    }

    // Base index into the shared output vertex array for this triangle's 3 vertices.
    const uint firstVertexIndex = triangleIndex * kVertexPerTriangle;

    // Write this triangle's 3 vertices:
    // - position is the shared triangle shape translated by this triangle's screen-space offset.
    // - color is picked from the palette with a per-triangle rotation so adjacent triangles don't look identical.
    for (uint i = 0U; i < kVertexPerTriangle; ++i) {
        const uint vertexIndex = firstVertexIndex + i;
        gl_MeshVerticesEXT[vertexIndex].gl_Position = vec4(kTriangleShape[i] + kTriangleOffsets[triangleIndex], 0.0, 1.0);
        outColor[vertexIndex] = kVertexColors[(triangleIndex + i) % kVertexPerTriangle];
    }

    // Declare the primitive (triangle) this thread owns, indexing into the vertex array written above.
    // Each thread writes exactly one entry at its own triangleIndex slot.
    gl_PrimitiveTriangleIndicesEXT[triangleIndex] = uvec3(firstVertexIndex, firstVertexIndex + 1U, firstVertexIndex + 2U);
}
