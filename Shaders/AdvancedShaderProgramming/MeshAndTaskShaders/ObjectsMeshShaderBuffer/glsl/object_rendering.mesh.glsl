#version 450
#extension GL_EXT_mesh_shader : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

// Number of triangles processed by a single mesh shader workgroup
#define kTrianglesPerWorkgroup 64U

layout(local_size_x = kTrianglesPerWorkgroup) in;
layout(triangles, max_vertices = kTrianglesPerWorkgroup * 3, max_primitives = kTrianglesPerWorkgroup) out;

layout(location = 0) out vec3 outWorldPos[];
layout(location = 1) out vec3 outWorldNormal[];

// Global vertex attribute buffers
layout(std430, set = 0, binding = 0) readonly buffer PositionBuffer { float positions[]; };
layout(std430, set = 0, binding = 1) readonly buffer NormalBuffer { float normals[]; };
layout(std430, set = 0, binding = 2) readonly buffer IndexBuffer { uint indices[]; };

// Scene-wide rendering parameters that contains common camera, light and material informations
layout(std140, set = 0, binding = 3) uniform SceneUBO {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    vec4 lightDirection;
    vec4 lightColor;
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
} scene;

// Per-draw data describing the mesh instance being rendered
layout(push_constant) uniform MeshPushConstantsBlock {
    mat4 model;
    mat4 invModel;
    uint vertexOffset; // Offset into the global vertex buffers
    uint vertexCount; // Number of vertices belonging to this mesh
    uint indexOffset; // Offset into the global index buffer
    uint primitiveCount; // Number of triangles in this mesh
} pc;

// Reads a packed float3 position from the position buffer
vec3 FetchPosition(uint i)
{
    const uint b = i * 3U;
    return vec3(positions[b], positions[b+1U], positions[b+2U]);
}

// Reads a packed float3 normal from the normal buffer
vec3 FetchNormal(uint i)
{
    const uint b = i * 3U;
    return vec3(normals[b],   normals[b+1U],   normals[b+2U]);
}

void main()
{
    // First triangle assigned to this workgroup
    const uint triangleBase = gl_WorkGroupID.x * kTrianglesPerWorkgroup;

    // If no triangles remain for this workgroup to process, declare zero mesh outputs and terminate the workgroup
    if (triangleBase >= pc.primitiveCount) {
        SetMeshOutputsEXT(0, 0);
        return;
    }

    // Last workgroup may process fewer than kTrianglesPerWorkgroup triangles
    const uint trianglesActual = min(uint(kTrianglesPerWorkgroup), pc.primitiveCount - triangleBase);

    // Declare the actual number of mesh vertices and primitives generated
    SetMeshOutputsEXT(trianglesActual * 3U, trianglesActual);

    // One invocation is responsible for one triangle
    const uint localTri = gl_LocalInvocationIndex;

    // Extra invocations in the final workgroup do nothing
    if (localTri >= trianglesActual) {
        return;
    }

    // Triangle index within the mesh
    const uint globalTri = triangleBase + localTri;

    // Output vertex range reserved for this triangle
    const uint firstVertex = localTri * 3U;

    // Precompute transforms once per invocation
    const mat4 modelViewProj = scene.proj * scene.view * pc.model;
    const mat3 normalMatrix = mat3(transpose(pc.invModel));

    // Emit the triangle's three vertices
    for (uint i = 0U; i < 3U; ++i) {
        const uint localVertexIndex = indices[pc.indexOffset + globalTri * 3U + i];
        const uint globalVertexIndex = pc.vertexOffset + localVertexIndex;

        const vec3 localPos = FetchPosition(globalVertexIndex);
        const vec3 localNormal = FetchNormal(globalVertexIndex);
        const vec4 worldPos = pc.model * vec4(localPos, 1.0);

        // World-space attributes passed to the fragment shader
        gl_MeshVerticesEXT[firstVertex + i].gl_Position = modelViewProj * vec4(localPos, 1.0);
        outWorldPos[firstVertex + i] = worldPos.xyz;
        outWorldNormal[firstVertex + i] = normalize(normalMatrix * localNormal);
    }

    // Define the primitive using the three emitted vertices
    gl_PrimitiveTriangleIndicesEXT[localTri] = uvec3(firstVertex, firstVertex + 1U, firstVertex + 2U);
}
