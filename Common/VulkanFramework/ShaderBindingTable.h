/**
 * @file    ShaderBindingTable.h
 * @brief   This file contains the implementation of the ShaderBindingTable class, which creates the shader binding
 *          table (SBT) buffer of a ray tracing pipeline and calculates its ray generation, miss, hit and callable
 *          regions according to the alignment rules of the device.
 * @author  Mustafa Yemural (myemural)
 * @date    28.11.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "BufferResource.h"
#include "CoreDefines.h"
#include "VulkanDevice.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanPipeline.h"

namespace common::vulkan_framework
{
enum class ShaderBindingTableRegion
{
    RAY_GEN,
    MISS,
    HIT,
    CALLABLE
};

/**
 * @brief Describes which shader groups of the pipeline are placed into which SBT region. Group indices are the indices
 *        of the shader groups in the order they are added to the ray tracing pipeline.
 */
struct COMMON_API ShaderBindingTableCreateInfo
{
    std::uint32_t rayGenGroupIndex = 0;
    std::vector<std::uint32_t> missGroupIndices;
    std::vector<std::uint32_t> hitGroupIndices;
    std::vector<std::uint32_t> callableGroupIndices;

    // Optional shader record data size (in bytes) that is placed after each group handle in the region
    std::uint32_t missRecordDataSize = 0;
    std::uint32_t hitRecordDataSize = 0;
    std::uint32_t callableRecordDataSize = 0;
};

class COMMON_API ShaderBindingTable
{
public:
    /**
     * @param physicalDevice Refers VulkanPhysicalDevice object.
     * @param device Refers VulkanDevice object.
     */
    ShaderBindingTable(const std::shared_ptr<vulkan_wrapper::VulkanPhysicalDevice>& physicalDevice,
                       const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device);

    /**
     * @brief Gets the shader group handles from the pipeline, creates the SBT buffer and copies the handles into the
     *        regions.
     * @param pipeline Ray tracing pipeline that the SBT belongs to.
     * @param pipelineGroupCount Total shader group count of the ray tracing pipeline.
     * @param createInfo Shader binding table create information.
     */
    void Create(const std::shared_ptr<vulkan_wrapper::VulkanPipeline>& pipeline,
                std::uint32_t pipelineGroupCount,
                const ShaderBindingTableCreateInfo& createInfo);

    /**
     * @brief Writes shader record data after the group handle of a record.
     * @param region SBT region of the record (ray generation records have no data).
     * @param recordIndex Index of the record in the region.
     * @param data Data that will be copied.
     * @param dataSize Size of the data. Must be smaller than or equal to the record data size of the region.
     */
    void SetRecordData(ShaderBindingTableRegion region,
                       std::uint32_t recordIndex,
                       const void* data,
                       std::uint32_t dataSize) const;

    /**
     * @return Returns the strided device address region that will be passed to vkCmdTraceRaysKHR.
     */
    [[nodiscard]] const VkStridedDeviceAddressRegionKHR& GetRayGenRegion() const { return rayGenRegion_; }

    [[nodiscard]] const VkStridedDeviceAddressRegionKHR& GetMissRegion() const { return missRegion_; }

    [[nodiscard]] const VkStridedDeviceAddressRegionKHR& GetHitRegion() const { return hitRegion_; }

    [[nodiscard]] const VkStridedDeviceAddressRegionKHR& GetCallableRegion() const { return callableRegion_; }

    /**
     * @return Returns VulkanBuffer object of the SBT.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetBuffer() const;

private:
    struct RegionInfo
    {
        VkDeviceSize bufferOffset;    // Byte offset of the region from the start of the buffer
        VkDeviceSize stride;          // Byte size of a record
        std::uint32_t recordCount;    // Record count of the region
        std::uint32_t recordDataSize; // Maximum shader record data size of a record
    };

    /**
     * @brief Returns layout information of the specified region.
     * @param region SBT region.
     * @return Returns layout information of the region.
     */
    [[nodiscard]] RegionInfo GetRegionInfo(ShaderBindingTableRegion region) const;

    std::weak_ptr<vulkan_wrapper::VulkanPhysicalDevice> physicalDevice_;
    std::weak_ptr<vulkan_wrapper::VulkanDevice> device_;

    std::unique_ptr<BufferResource> buffer_ = nullptr;
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProperties_{};

    VkStridedDeviceAddressRegionKHR rayGenRegion_{};
    VkStridedDeviceAddressRegionKHR missRegion_{};
    VkStridedDeviceAddressRegionKHR hitRegion_{};
    VkStridedDeviceAddressRegionKHR callableRegion_{};

    // Byte offsets of the regions from the start of the buffer (includes the base alignment padding)
    VkDeviceSize rayGenOffset_ = 0;
    VkDeviceSize missOffset_ = 0;
    VkDeviceSize hitOffset_ = 0;
    VkDeviceSize callableOffset_ = 0;

    ShaderBindingTableCreateInfo createInfo_;
};
} // namespace common::vulkan_framework
