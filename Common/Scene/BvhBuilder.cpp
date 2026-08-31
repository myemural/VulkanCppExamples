/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "BvhBuilder.h"

#include <algorithm>
#include <cstring>
#include <limits>

#include <glm/gtc/matrix_inverse.hpp>

namespace common::scene
{

namespace
{
    constexpr auto kBinCount = 8;
    constexpr auto kFloatMax = std::numeric_limits<float>::max();

    struct Bin
    {
        glm::vec3 boundsMin{kFloatMax};
        glm::vec3 boundsMax{-kFloatMax};
        std::uint32_t count = 0U;
    };

    float SurfaceArea(const glm::vec3& boundsMin, const glm::vec3& boundsMax)
    {
        const glm::vec3 extent = glm::max(boundsMax - boundsMin, glm::vec3(0.0f));
        return extent.x * extent.y + extent.y * extent.z + extent.z * extent.x;
    }

    float AsFloatBits(const std::uint32_t value)
    {
        float result = 0.0f;
        std::memcpy(&result, &value, sizeof(result));
        return result;
    }

    template<typename T>
    const T* AccessorData(const Accessor& accessor)
    {
        return reinterpret_cast<const T*>(accessor.bufferView.data + accessor.bufferView.byteOffset +
                                          accessor.byteOffset);
    }

    void GrowBounds(glm::vec3& boundsMin, glm::vec3& boundsMax, const TriangleGpu& triangle)
    {
        boundsMin = glm::min(
                boundsMin, glm::min(glm::vec3(triangle.v0), glm::min(glm::vec3(triangle.v1), glm::vec3(triangle.v2))));
        boundsMax = glm::max(
                boundsMax, glm::max(glm::vec3(triangle.v0), glm::max(glm::vec3(triangle.v1), glm::vec3(triangle.v2))));
    }

} // namespace

void BvhBuilder::AddMesh(const MeshPrimitive& meshPrimitive, const glm::mat4& worldMatrix, const std::uint32_t objectId)
{
    const auto& posAccessor = meshPrimitive.attributes.at(AttributeType::POSITION);
    const auto& normalAccessor = meshPrimitive.attributes.at(AttributeType::NORMAL);

    const auto* positions = AccessorData<glm::vec3>(posAccessor);
    const auto* normals = AccessorData<glm::vec3>(normalAccessor);
    const auto* indices = AccessorData<std::uint16_t>(meshPrimitive.indices);

    const auto normalMatrix = glm::mat3(glm::inverseTranspose(worldMatrix));
    const auto packedObjectId = AsFloatBits(objectId);

    triangles_.reserve(triangles_.size() + meshPrimitive.indices.count / 3);

    for (auto i = 0U; i + 2 < meshPrimitive.indices.count; i += 3) {
        glm::vec3 vertices[3];
        glm::vec3 vertexNormals[3];

        for (auto k = 0U; k < 3; ++k) {
            const auto index = indices[i + k];
            vertices[k] = glm::vec3(worldMatrix * glm::vec4(positions[index], 1.0f));
            vertexNormals[k] = glm::normalize(normalMatrix * normals[index]);
        }

        TriangleGpu triangle{};
        triangle.v0 = glm::vec4(vertices[0], packedObjectId);
        triangle.v1 = glm::vec4(vertices[1], 0.0f);
        triangle.v2 = glm::vec4(vertices[2], 0.0f);
        triangle.n0 = glm::vec4(vertexNormals[0], 0.0f);
        triangle.n1 = glm::vec4(vertexNormals[1], 0.0f);
        triangle.n2 = glm::vec4(vertexNormals[2], 0.0f);

        triangles_.push_back(triangle);
        centroids_.push_back((vertices[0] + vertices[1] + vertices[2]) / 3.0f);
    }
}

void BvhBuilder::Build(const std::uint32_t maxLeafSize)
{
    nodes_.clear();
    if (triangles_.empty()) {
        return;
    }

    nodes_.reserve(triangles_.size() * 2);
    nodes_.emplace_back();
    nodes_[0].leftFirst = 0U;
    nodes_[0].triangleCount = static_cast<std::uint32_t>(triangles_.size());

    UpdateNodeBounds(0U);
    Subdivide(0U, maxLeafSize);
}

void BvhBuilder::UpdateNodeBounds(const std::uint32_t nodeIndex)
{
    auto& node = nodes_[nodeIndex];
    node.boundsMin = glm::vec3(kFloatMax);
    node.boundsMax = glm::vec3(-kFloatMax);

    for (std::uint32_t i = 0U; i < node.triangleCount; ++i) {
        GrowBounds(node.boundsMin, node.boundsMax, triangles_[node.leftFirst + i]);
    }
}

void BvhBuilder::Subdivide(const std::uint32_t nodeIndex, const std::uint32_t maxLeafSize)
{
    const auto first = nodes_[nodeIndex].leftFirst;
    const auto count = nodes_[nodeIndex].triangleCount;

    if (count <= maxLeafSize) {
        return;
    }

    // Centroid bounds drive the binning, they give much tighter splits that the full triangle bounds
    glm::vec3 centroidMin{kFloatMax};
    glm::vec3 centroidMax{-kFloatMax};
    for (std::uint32_t i = 0U; i < count; ++i) {
        centroidMin = glm::min(centroidMin, centroids_[first + i]);
        centroidMax = glm::max(centroidMax, centroids_[first + i]);
    }

    int bestAxis = -1;
    float bestPosition = 0.0f;
    float bestCost = kFloatMax;

    for (int axis = 0; axis < 3; ++axis) {
        const float low = centroidMin[axis];
        const float high = centroidMax[axis];
        if (high - low < 1e-6f) {
            continue;
        }

        Bin bins[kBinCount];
        const float scale = static_cast<float>(kBinCount) / (high - low);

        for (std::uint32_t i = 0U; i < count; ++i) {
            const auto binIndex =
                    std::min(kBinCount - 1, static_cast<int>((centroids_[first + i][axis] - low) * scale));

            auto& bin = bins[binIndex];
            ++bin.count;
            GrowBounds(bin.boundsMin, bin.boundsMax, triangles_[first + i]);
        }

        // Sweep from both sides so every split plane cost is a single array lookup
        float leftAreas[kBinCount - 1];
        float rightAreas[kBinCount - 1];
        std::uint32_t leftCounts[kBinCount - 1];
        std::uint32_t rightCounts[kBinCount - 1];

        glm::vec3 accumulateMin{kFloatMax};
        glm::vec3 accumulateMax{-kFloatMax};
        std::uint32_t accumulateCount = 0;
        for (int i = 0; i < kBinCount - 1; ++i) {
            accumulateMin = glm::min(accumulateMin, bins[i].boundsMin);
            accumulateMax = glm::max(accumulateMax, bins[i].boundsMax);
            accumulateCount += bins[i].count;
            leftAreas[i] = SurfaceArea(accumulateMin, accumulateMax);
            leftCounts[i] = accumulateCount;
        }

        accumulateMin = glm::vec3{kFloatMax};
        accumulateMax = glm::vec3{-kFloatMax};
        accumulateCount = 0;
        for (int i = kBinCount - 1; i > 0; --i) {
            accumulateMin = glm::min(accumulateMin, bins[i].boundsMin);
            accumulateMax = glm::max(accumulateMax, bins[i].boundsMax);
            accumulateCount += bins[i].count;
            rightAreas[i - 1] = SurfaceArea(accumulateMin, accumulateMax);
            rightCounts[i - 1] = accumulateCount;
        }

        for (int i = 0; i < kBinCount - 1; ++i) {
            if (leftCounts[i] == 0 || rightCounts[i] == 0) {
                continue;
            }

            if (const float cost = static_cast<float>(leftCounts[i]) * leftAreas[i] +
                                   static_cast<float>(rightCounts[i]) * rightAreas[i];
                cost < bestCost) {
                bestCost = cost;
                bestAxis = axis;
                bestPosition = low + static_cast<float>(i + 1) / scale;
            }
        }
    }

    // Splitting is only worth it when it beats the cost of keeping the node as a leaf
    const float leafCost =
            static_cast<float>(count) * SurfaceArea(nodes_[nodeIndex].boundsMin, nodes_[nodeIndex].boundsMax);
    if (bestAxis < 0 || bestCost >= leafCost) {
        return;
    }

    // In place partition around the split plane
    auto left = static_cast<std::int64_t>(first);
    auto right = static_cast<std::int64_t>(first + count) - 1;
    while (left <= right) {
        if (centroids_[left][bestAxis] < bestPosition) {
            ++left;
        } else {
            std::swap(triangles_[left], triangles_[right]);
            std::swap(centroids_[left], centroids_[right]);
            --right;
        }
    }

    const auto leftCount = static_cast<std::uint32_t>(left) - first;
    if (leftCount == 0 || leftCount == count) {
        return;
    }

    const auto leftChild = static_cast<std::uint32_t>(nodes_.size());
    nodes_.emplace_back();
    nodes_.emplace_back();

    nodes_[leftChild].leftFirst = first;
    nodes_[leftChild].triangleCount = leftCount;
    nodes_[leftChild + 1].leftFirst = first + leftCount;
    nodes_[leftChild + 1].triangleCount = count - leftCount;

    nodes_[nodeIndex].leftFirst = leftChild;
    nodes_[nodeIndex].triangleCount = 0;

    UpdateNodeBounds(leftChild);
    UpdateNodeBounds(leftChild + 1);

    Subdivide(leftChild, maxLeafSize);
    Subdivide(leftChild + 1, maxLeafSize);
}

} // namespace common::scene
