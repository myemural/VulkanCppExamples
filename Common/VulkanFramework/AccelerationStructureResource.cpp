/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "AccelerationStructureResource.h"

#include <algorithm>

#include "MathUtils.h"

namespace common::vulkan_framework
{
using namespace common::utility;
using namespace common::vulkan_wrapper;

AccelerationStructureResource::AccelerationStructureResource(
        const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice, const std::shared_ptr<VulkanDevice>& device)
    : physicalDevice_{physicalDevice}, device_{device}, createInfo_{}
{
}

void
AccelerationStructureResource::CreateAccelerationStructure(const AccelerationStructureResourceCreateInfo& createInfo)
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

    ValidateCreateInfo();
    FillGeometries();

    // Query sizes with the maximum primitive counts (the build can use less or equal primitives later)
    const auto sizeQueryInfo = GenerateBuildGeometryInfo(VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR);
    buildSizes_ = devicePtr->GetAccelerationStructureBuildSizes(sizeQueryInfo, maxPrimitiveCounts_);

    // Storage buffer that will keep the acceleration structure data
    storageBuffer_ = std::make_unique<BufferResource>(physicalDevicePtr, devicePtr);
    storageBuffer_->CreateBuffer(
            {createInfo_.name + "_storageBuffer", buildSizes_.accelerationStructureSize,
             VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT});

    accelerationStructure_ = devicePtr->CreateAccelerationStructure(storageBuffer_->GetBuffer(), createInfo_.type,
                                                                    buildSizes_.accelerationStructureSize);
    if (!accelerationStructure_) {
        throw std::runtime_error("Failed to create acceleration structure \"" + createInfo_.name + "\"!");
    }

    // Scratch buffer is used as a temporary memory while building. It is kept alive, so the acceleration structure can
    // be rebuilt or updated later. Its address must be aligned to minAccelerationStructureScratchOffsetAlignment.
    VkPhysicalDeviceAccelerationStructurePropertiesKHR asProperties{};
    asProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
    physicalDevicePtr->GetProperties2(&asProperties);

    const VkDeviceSize scratchAlignment =
            std::max<VkDeviceSize>(asProperties.minAccelerationStructureScratchOffsetAlignment, 1);
    const VkDeviceSize scratchSize = std::max(buildSizes_.buildScratchSize, buildSizes_.updateScratchSize);

    scratchBuffer_ = std::make_unique<BufferResource>(physicalDevicePtr, devicePtr);
    scratchBuffer_->CreateBuffer({createInfo_.name + "_scratchBuffer", scratchSize + scratchAlignment,
                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT});

    scratchAddress_ = AlignUp(scratchBuffer_->GetDeviceAddress(), scratchAlignment);
    isBuilt_ = false;
}

void AccelerationStructureResource::RecordBuild(const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer,
                                                const VkBuildAccelerationStructureModeKHR mode)
{
    if (!accelerationStructure_) {
        throw std::runtime_error("Acceleration structure must be created before recording its build!");
    }

    if (mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR) {
        if (!(createInfo_.buildFlags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR)) {
            throw std::runtime_error("Acceleration structure \"" + createInfo_.name +
                                     "\" was not created with ALLOW_UPDATE flag!");
        }
        if (!isBuilt_) {
            throw std::runtime_error("Acceleration structure \"" + createInfo_.name +
                                     "\" must be built before it is updated!");
        }
    }

    const auto buildInfo = GenerateBuildGeometryInfo(mode);
    cmdBuffer->BuildAccelerationStructuresKHR({buildInfo}, {buildRanges_.data()});

    isBuilt_ = true;
}

void AccelerationStructureResource::ValidateCreateInfo() const
{
    const bool hasTriangles = !createInfo_.triangleGeometries.empty();
    const bool hasAabbs = !createInfo_.aabbGeometries.empty();
    const bool hasInstances = createInfo_.instanceGeometry.has_value();

    if (createInfo_.type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
        if (hasInstances) {
            throw std::runtime_error("Bottom-level acceleration structure cannot contain instances!");
        }
        if (hasTriangles == hasAabbs) {
            throw std::runtime_error(
                    "Bottom-level acceleration structure must contain either triangle or AABB geometries!");
        }
    } else if (createInfo_.type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
        if (!hasInstances || hasTriangles || hasAabbs) {
            throw std::runtime_error("Top-level acceleration structure must contain only the instance geometry!");
        }
    } else {
        throw std::runtime_error("Unsupported acceleration structure type!");
    }
}

void AccelerationStructureResource::FillGeometries()
{
    geometries_.clear();
    buildRanges_.clear();
    maxPrimitiveCounts_.clear();

    for (const auto& triangles: createInfo_.triangleGeometries) {
        VkAccelerationStructureGeometryKHR geometry{};
        geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometry.pNext = nullptr;
        geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        geometry.flags = triangles.flags;
        geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        geometry.geometry.triangles.pNext = nullptr;
        geometry.geometry.triangles.vertexFormat = triangles.vertexFormat;
        geometry.geometry.triangles.vertexData.deviceAddress = triangles.vertexBufferAddress;
        geometry.geometry.triangles.vertexStride = triangles.vertexStride;
        geometry.geometry.triangles.maxVertex = triangles.maxVertex;
        geometry.geometry.triangles.indexType =
                triangles.indexBufferAddress != 0 ? triangles.indexType : VK_INDEX_TYPE_NONE_KHR;
        geometry.geometry.triangles.indexData.deviceAddress = triangles.indexBufferAddress;
        geometry.geometry.triangles.transformData.deviceAddress = triangles.transformBufferAddress;
        geometries_.push_back(geometry);

        VkAccelerationStructureBuildRangeInfoKHR buildRange{};
        buildRange.primitiveCount = triangles.primitiveCount;
        buildRange.primitiveOffset = triangles.primitiveOffset;
        buildRange.firstVertex = triangles.firstVertex;
        buildRange.transformOffset = triangles.transformOffset;
        buildRanges_.push_back(buildRange);

        maxPrimitiveCounts_.push_back(triangles.primitiveCount);
    }

    for (const auto& aabbs: createInfo_.aabbGeometries) {
        VkAccelerationStructureGeometryKHR geometry{};
        geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometry.pNext = nullptr;
        geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
        geometry.flags = aabbs.flags;
        geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
        geometry.geometry.aabbs.pNext = nullptr;
        geometry.geometry.aabbs.data.deviceAddress = aabbs.aabbBufferAddress;
        geometry.geometry.aabbs.stride = aabbs.stride;
        geometries_.push_back(geometry);

        VkAccelerationStructureBuildRangeInfoKHR buildRange{};
        buildRange.primitiveCount = aabbs.primitiveCount;
        buildRange.primitiveOffset = aabbs.primitiveOffset;
        buildRange.firstVertex = 0;
        buildRange.transformOffset = 0;
        buildRanges_.push_back(buildRange);

        maxPrimitiveCounts_.push_back(aabbs.primitiveCount);
    }

    if (createInfo_.instanceGeometry.has_value()) {
        const auto& instances = createInfo_.instanceGeometry.value();

        VkAccelerationStructureGeometryKHR geometry{};
        geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometry.pNext = nullptr;
        geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        geometry.flags = instances.flags;
        geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        geometry.geometry.instances.pNext = nullptr;
        geometry.geometry.instances.arrayOfPointers = VK_FALSE;
        geometry.geometry.instances.data.deviceAddress = instances.instanceBufferAddress;
        geometries_.push_back(geometry);

        VkAccelerationStructureBuildRangeInfoKHR buildRange{};
        buildRange.primitiveCount = instances.instanceCount;
        buildRange.primitiveOffset = 0;
        buildRange.firstVertex = 0;
        buildRange.transformOffset = 0;
        buildRanges_.push_back(buildRange);

        maxPrimitiveCounts_.push_back(instances.instanceCount);
    }
}

VkAccelerationStructureBuildGeometryInfoKHR
AccelerationStructureResource::GenerateBuildGeometryInfo(const VkBuildAccelerationStructureModeKHR mode) const
{
    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.type = createInfo_.type;
    buildInfo.flags = createInfo_.buildFlags;
    buildInfo.mode = mode;
    buildInfo.geometryCount = static_cast<std::uint32_t>(geometries_.size());
    buildInfo.pGeometries = geometries_.data();
    buildInfo.ppGeometries = nullptr;

    // Handles and the scratch address are ignored while querying the build sizes
    const VkAccelerationStructureKHR handle =
            accelerationStructure_ ? accelerationStructure_->GetHandle() : VK_NULL_HANDLE;
    buildInfo.srcAccelerationStructure =
            mode == VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR ? handle : VK_NULL_HANDLE; // In-place update
    buildInfo.dstAccelerationStructure = handle;
    buildInfo.scratchData.deviceAddress = scratchAddress_;

    return buildInfo;
}
} // namespace common::vulkan_framework
