/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ShaderBindingTable.h"

#include <algorithm>
#include <cstring>

#include "MathUtils.h"

namespace common::vulkan_framework
{
using namespace common::utility;
using namespace common::vulkan_wrapper;

ShaderBindingTable::ShaderBindingTable(const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice,
                                       const std::shared_ptr<VulkanDevice>& device)
    : physicalDevice_{physicalDevice}, device_{device}
{
}

void ShaderBindingTable::Create(const std::shared_ptr<VulkanPipeline>& pipeline,
                                const std::uint32_t pipelineGroupCount,
                                const ShaderBindingTableCreateInfo& createInfo)
{
    createInfo_ = createInfo;

    const auto devicePtr = device_.lock();
    if (!devicePtr) {
        throw std::runtime_error("Device object not found!");
    }

    const auto physicalDevicePtr = physicalDevice_.lock();
    if (!physicalDevicePtr) {
        throw std::runtime_error("Physical device object not found!");
    }

    // Check that all group indices are inside the pipeline
    const auto isOutOfRange = [&](const std::uint32_t groupIndex) { return groupIndex >= pipelineGroupCount; };
    if (isOutOfRange(createInfo_.rayGenGroupIndex) || std::ranges::any_of(createInfo_.missGroupIndices, isOutOfRange) ||
        std::ranges::any_of(createInfo_.hitGroupIndices, isOutOfRange) ||
        std::ranges::any_of(createInfo_.callableGroupIndices, isOutOfRange)) {
        throw std::runtime_error("Shader group index is out of the pipeline group range!");
    }

    // Query the alignment rules of the device
    rayTracingProperties_ = {};
    rayTracingProperties_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    rayTracingProperties_.pNext = nullptr;
    physicalDevicePtr->GetProperties2(&rayTracingProperties_);

    const VkDeviceSize handleSize = rayTracingProperties_.shaderGroupHandleSize;
    const VkDeviceSize handleAlignment = rayTracingProperties_.shaderGroupHandleAlignment;
    const VkDeviceSize baseAlignment = rayTracingProperties_.shaderGroupBaseAlignment;

    // Each record is a group handle followed by the optional shader record data. Stride of a region must be a multiple
    // of shaderGroupHandleAlignment and start address of a region must be a multiple of shaderGroupBaseAlignment.
    const auto calculateRegion = [&](const std::size_t recordCount, const std::uint32_t recordDataSize) {
        VkStridedDeviceAddressRegionKHR region{};
        if (recordCount > 0) {
            region.stride = AlignUp(handleSize + recordDataSize, handleAlignment);
            region.size = AlignUp(region.stride * recordCount, baseAlignment);
        }
        return region;
    };

    // Size of the ray generation region must be equal to its stride
    rayGenRegion_ = {};
    rayGenRegion_.stride = AlignUp(handleSize, baseAlignment);
    rayGenRegion_.size = rayGenRegion_.stride;
    missRegion_ = calculateRegion(createInfo_.missGroupIndices.size(), createInfo_.missRecordDataSize);
    hitRegion_ = calculateRegion(createInfo_.hitGroupIndices.size(), createInfo_.hitRecordDataSize);
    callableRegion_ = calculateRegion(createInfo_.callableGroupIndices.size(), createInfo_.callableRecordDataSize);

    const VkDeviceSize maxStride = rayTracingProperties_.maxShaderGroupStride;
    if (missRegion_.stride > maxStride || hitRegion_.stride > maxStride || callableRegion_.stride > maxStride) {
        throw std::runtime_error("Shader record size exceeds the maxShaderGroupStride limit of the device!");
    }

    // Create a host-visible buffer. Extra base alignment bytes are allocated, because the buffer address itself is not
    // guaranteed to be aligned to shaderGroupBaseAlignment.
    const VkDeviceSize tableSize = rayGenRegion_.size + missRegion_.size + hitRegion_.size + callableRegion_.size;

    buffer_ = std::make_unique<BufferResource>(physicalDevicePtr, devicePtr);
    buffer_->CreateBuffer({"shaderBindingTableBuffer", tableSize + baseAlignment,
                           VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT});

    const VkDeviceAddress bufferAddress = buffer_->GetDeviceAddress();
    const VkDeviceAddress alignedAddress = AlignUp(bufferAddress, baseAlignment);
    const VkDeviceSize padding = alignedAddress - bufferAddress;

    rayGenOffset_ = padding;
    missOffset_ = rayGenOffset_ + rayGenRegion_.size;
    hitOffset_ = missOffset_ + missRegion_.size;
    callableOffset_ = hitOffset_ + hitRegion_.size;

    // Empty regions must have zero device address
    rayGenRegion_.deviceAddress = bufferAddress + rayGenOffset_;
    missRegion_.deviceAddress = missRegion_.size > 0 ? bufferAddress + missOffset_ : 0;
    hitRegion_.deviceAddress = hitRegion_.size > 0 ? bufferAddress + hitOffset_ : 0;
    callableRegion_.deviceAddress = callableRegion_.size > 0 ? bufferAddress + callableOffset_ : 0;

    // Copy group handles into their records. Memory is kept mapped for later shader record data updates.
    const auto handles =
            pipeline->GetRayTracingShaderGroupHandles(0, pipelineGroupCount, pipelineGroupCount * handleSize);

    buffer_->MapMemory();
    auto* mappedData = static_cast<std::uint8_t*>(buffer_->GetMappedData());
    std::memset(mappedData, 0, tableSize + baseAlignment);

    const auto copyHandles = [&](const std::vector<std::uint32_t>& groupIndices, const VkDeviceSize regionOffset,
                                 const VkDeviceSize stride) {
        for (std::size_t i = 0; i < groupIndices.size(); ++i) {
            std::memcpy(mappedData + regionOffset + i * stride, handles.data() + groupIndices[i] * handleSize,
                        handleSize);
        }
    };

    copyHandles({createInfo_.rayGenGroupIndex}, rayGenOffset_, rayGenRegion_.stride);
    copyHandles(createInfo_.missGroupIndices, missOffset_, missRegion_.stride);
    copyHandles(createInfo_.hitGroupIndices, hitOffset_, hitRegion_.stride);
    copyHandles(createInfo_.callableGroupIndices, callableOffset_, callableRegion_.stride);
}

void ShaderBindingTable::SetRecordData(const ShaderBindingTableRegion region,
                                       const std::uint32_t recordIndex,
                                       const void* data,
                                       const std::uint32_t dataSize) const
{
    if (!buffer_) {
        throw std::runtime_error("Shader binding table must be created before setting record data!");
    }

    const auto regionInfo = GetRegionInfo(region);
    if (recordIndex >= regionInfo.recordCount) {
        throw std::runtime_error("Shader record index is out of the region range!");
    }

    if (dataSize > regionInfo.recordDataSize) {
        throw std::runtime_error("Shader record data size is bigger than the record data size of the region!");
    }

    auto* mappedData = static_cast<std::uint8_t*>(buffer_->GetMappedData());
    const VkDeviceSize dataOffset =
            regionInfo.bufferOffset + recordIndex * regionInfo.stride + rayTracingProperties_.shaderGroupHandleSize;
    std::memcpy(mappedData + dataOffset, data, dataSize);
}

std::shared_ptr<VulkanBuffer> ShaderBindingTable::GetBuffer() const { return buffer_ ? buffer_->GetBuffer() : nullptr; }

ShaderBindingTable::RegionInfo ShaderBindingTable::GetRegionInfo(const ShaderBindingTableRegion region) const
{
    switch (region) {
        case ShaderBindingTableRegion::RAY_GEN:
            return {rayGenOffset_, rayGenRegion_.stride, 1, 0};
        case ShaderBindingTableRegion::MISS:
            return {missOffset_, missRegion_.stride, static_cast<std::uint32_t>(createInfo_.missGroupIndices.size()),
                    createInfo_.missRecordDataSize};
        case ShaderBindingTableRegion::HIT:
            return {hitOffset_, hitRegion_.stride, static_cast<std::uint32_t>(createInfo_.hitGroupIndices.size()),
                    createInfo_.hitRecordDataSize};
        case ShaderBindingTableRegion::CALLABLE:
            return {callableOffset_, callableRegion_.stride,
                    static_cast<std::uint32_t>(createInfo_.callableGroupIndices.size()),
                    createInfo_.callableRecordDataSize};
        default:
            throw std::invalid_argument("Unknown shader binding table region!");
    }
}
} // namespace common::vulkan_framework
