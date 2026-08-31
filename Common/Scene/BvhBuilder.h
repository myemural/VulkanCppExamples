/**
 * @file    BvhBuilder.h
 * @brief   CPU-side binned-SAH BVH builder that produces GPU-ready node and triangle arrays
 * @author  Mustafa Yemural (myemural)
 * @date    30.08.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "SceneMesh.h"

namespace common::scene
{

struct alignas(16) BvhNodeGpu
{
    glm::vec3 boundsMin{0.0f};
    std::uint32_t leftFirst = 0U;     // For Inner node: Left child index, For Leaf: First triangle index
    glm::vec3 boundsMax{0.0f};
    std::uint32_t triangleCount = 0U; // 0 marks an inner node
};

struct alignas(16) TriangleGpu
{
    glm::vec4 v0{0.0f}; // xyz = Position, w = Owning Object ID (bit pattern)
    glm::vec4 v1{0.0f};
    glm::vec4 v2{0.0f};
    glm::vec4 n0{0.0f};
    glm::vec4 n1{0.0f};
    glm::vec4 n2{0.0f};
};

class COMMON_API BvhBuilder
{
public:
    /**
     * @brief Bakes a mesh primitive into world-space and appends its triangles to the build input.
     * @param meshPrimitive Mesh primitive with position, normal and index accessors.
     * @param worldMatrix World matrix of the owning scene object.
     * @param objectId Object ID used to look the material up in the scene material storage buffer.
     */
    void AddMesh(const MeshPrimitive& meshPrimitive, const glm::mat4& worldMatrix, std::uint32_t objectId);

    /**
     * @brief Builds the BVH. Triangles are reordered in place so that every leaf owns a contiguous range.
     * @param maxLeafSize Triangle count under which a node is not split any further.
     */
    void Build(std::uint32_t maxLeafSize);

    [[nodiscard]] const std::vector<BvhNodeGpu>& GetNodes() const { return nodes_; }

    [[nodiscard]] const std::vector<TriangleGpu>& GetTriangles() const { return triangles_; }

private:
    void UpdateNodeBounds(std::uint32_t nodeIndex);

    void Subdivide(std::uint32_t nodeIndex, std::uint32_t maxLeafSize);

    std::vector<TriangleGpu> triangles_;
    std::vector<glm::vec3> centroids_;
    std::vector<BvhNodeGpu> nodes_;
};

} // namespace common::scene
