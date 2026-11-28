/**
 * @file    AccelerationStructureResource.h
 * @brief   This file contains the implementation of the AccelerationStructureResource class, which performs operations
 *          related to creating a ray tracing acceleration structure (BLAS or TLAS), allocating its storage and scratch
 *          buffers, and recording its build commands.
 * @author  Mustafa Yemural (myemural)
 * @date    28.11.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "BufferResource.h"
#include "CoreDefines.h"
#include "VulkanAccelerationStructure.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanPhysicalDevice.h"

namespace common::vulkan_framework
{
/**
 * @brief Triangle geometry of a bottom-level acceleration structure. All addresses are buffer device addresses and
 *        the buffers must be created with VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR and
 *        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT usage flags.
 */
struct COMMON_API AccelerationStructureTriangleGeometry
{
    VkDeviceAddress vertexBufferAddress = 0;
    VkFormat vertexFormat = VK_FORMAT_R32G32B32_SFLOAT; // Format of the vertex position
    VkDeviceSize vertexStride = 0;
    std::uint32_t maxVertex = 0;                        // Highest vertex index that can be addressed by the build
    VkDeviceAddress indexBufferAddress = 0;             // 0 means the geometry is not indexed
    VkIndexType indexType = VK_INDEX_TYPE_UINT32;
    VkDeviceAddress transformBufferAddress = 0;         // Optional VkTransformMatrixKHR, 0 means identity
    std::uint32_t primitiveCount = 0;                   // Triangle count
    std::uint32_t primitiveOffset = 0; // Byte offset into the index buffer (or vertex buffer if not indexed)
    std::uint32_t firstVertex = 0;     // Index of the first vertex
    std::uint32_t transformOffset = 0; // Byte offset into the transform buffer
    VkGeometryFlagsKHR flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
};

/**
 * @brief Procedural (AABB) geometry of a bottom-level acceleration structure. Each primitive is a
 *        VkAabbPositionsKHR in the buffer.
 */
struct COMMON_API AccelerationStructureAabbGeometry
{
    VkDeviceAddress aabbBufferAddress = 0;
    VkDeviceSize stride = sizeof(VkAabbPositionsKHR); // Must be a multiple of 8
    std::uint32_t primitiveCount = 0;                 // AABB count
    std::uint32_t primitiveOffset = 0;                // Byte offset into the AABB buffer
    VkGeometryFlagsKHR flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
};

/**
 * @brief Instance geometry of a top-level acceleration structure. The buffer contains tightly packed
 *        VkAccelerationStructureInstanceKHR elements.
 */
struct COMMON_API AccelerationStructureInstanceGeometry
{
    VkDeviceAddress instanceBufferAddress = 0; // Must be aligned to 16 bytes
    std::uint32_t instanceCount = 0;
    VkGeometryFlagsKHR flags = 0;
};

struct COMMON_API AccelerationStructureResourceCreateInfo
{
    std::string name;
    VkAccelerationStructureTypeKHR type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    VkBuildAccelerationStructureFlagsKHR buildFlags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

    // A BLAS contains either triangle geometries or AABB geometries, a TLAS contains only the instance geometry
    std::vector<AccelerationStructureTriangleGeometry> triangleGeometries;
    std::vector<AccelerationStructureAabbGeometry> aabbGeometries;
    std::optional<AccelerationStructureInstanceGeometry> instanceGeometry;
};

class COMMON_API AccelerationStructureResource
{
public:
    /**
     * @param physicalDevice Refers VulkanPhysicalDevice object.
     * @param device Refers VulkanDevice object.
     */
    AccelerationStructureResource(const std::shared_ptr<vulkan_wrapper::VulkanPhysicalDevice>& physicalDevice,
                                  const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device);

    /**
     * @brief Queries the build sizes, creates the storage and scratch buffers, and creates the acceleration structure
     *        object. The acceleration structure is not built until RecordBuild is called and submitted.
     * @param createInfo Acceleration structure create information.
     */
    void CreateAccelerationStructure(const AccelerationStructureResourceCreateInfo& createInfo);

    /**
     * @brief Records the build (or update) command of the acceleration structure into the command buffer. A barrier
     *        is needed between this command and the commands that read the acceleration structure.
     * @param cmdBuffer Command buffer that the build command will be recorded.
     * @param mode Specifies whether the acceleration structure will be built from scratch or updated. Update mode
     *             requires VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR flag and a previous build.
     */
    void RecordBuild(const std::shared_ptr<vulkan_wrapper::VulkanCommandBuffer>& cmdBuffer,
                     VkBuildAccelerationStructureModeKHR mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);

    /**
     * @return Returns VulkanAccelerationStructure object that held from this class.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanAccelerationStructure> GetAccelerationStructure() const
    {
        return accelerationStructure_;
    }

    /**
     * @return Returns the queried build sizes (storage, build scratch and update scratch sizes).
     */
    [[nodiscard]] const VkAccelerationStructureBuildSizesInfoKHR& GetBuildSizes() const { return buildSizes_; }

private:
    /**
     * @brief Checks the create information for invalid geometry combinations.
     */
    void ValidateCreateInfo() const;

    /**
     * @brief Converts create information into Vulkan geometry and build range structures.
     */
    void FillGeometries();

    /**
     * @brief Generates a build geometry info with the current geometries.
     * @param mode Build mode of the acceleration structure.
     * @return Returns the build geometry info.
     */
    [[nodiscard]] VkAccelerationStructureBuildGeometryInfoKHR
    GenerateBuildGeometryInfo(VkBuildAccelerationStructureModeKHR mode) const;

    std::weak_ptr<vulkan_wrapper::VulkanPhysicalDevice> physicalDevice_;
    std::weak_ptr<vulkan_wrapper::VulkanDevice> device_;

    AccelerationStructureResourceCreateInfo createInfo_;
    std::vector<VkAccelerationStructureGeometryKHR> geometries_;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges_;
    std::vector<std::uint32_t> maxPrimitiveCounts_;
    VkAccelerationStructureBuildSizesInfoKHR buildSizes_{};

    std::unique_ptr<BufferResource> storageBuffer_ = nullptr;
    std::unique_ptr<BufferResource> scratchBuffer_ = nullptr;
    VkDeviceAddress scratchAddress_ = 0;
    std::shared_ptr<vulkan_wrapper::VulkanAccelerationStructure> accelerationStructure_ = nullptr;
    bool isBuilt_ = false;
};
} // namespace common::vulkan_framework
