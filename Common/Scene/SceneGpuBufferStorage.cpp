/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "SceneGpuBufferStorage.h"

#include <utility>

namespace common::scene
{

SceneGpuBufferStorage::SceneGpuBufferStorage(vulkan_framework::ResourceManager& resourceManager,
                                             const SceneConfig& sceneConfig)
    : resourceManager_{resourceManager}, sceneConfig_(sceneConfig)
{
    vulkan_framework::ResourceDescriptor resourceCreateInfo;

    resourceCreateInfo.buffers = {{kGeometryBufferName, kBufferSizeInBytes,
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kTransformStorageBufferName, kBufferSizeInBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kMaterialStorageBufferName, kBufferSizeInBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    if (resourceCreateInfo.buffers.has_value()) {
        resourceManager_.CreateBuffers(resourceCreateInfo.buffers.value());
    }
}

MeshGpu SceneGpuBufferStorage::AllocateMesh(const MeshPrimitive& meshPrimitive)
{
    return AllocateMeshGpuInternal(meshPrimitive);
}

MeshGpu SceneGpuBufferStorage::AllocateBuiltinMesh(const BuiltinMeshType& builtinMeshType)
{
    if (const auto meshName = GetBuiltinMeshName(builtinMeshType); meshCache_.contains(meshName)) {
        return meshCache_[meshName];
    }

    MeshGpu meshGpu;
    switch (builtinMeshType) {
        case BuiltinMeshType::CUBE: {
            const auto cubePrimitive = CubePrimitive{1.0f};
            meshGpu = AllocateMesh(cubePrimitive.GetMeshPrimitive());
            break;
        }
        case BuiltinMeshType::SPHERE: {
            const auto spherePrimitive =
                    SpherePrimitive{1.0f, sceneConfig_.primitiveStackCount, sceneConfig_.primitiveSectorCount};
            meshGpu = AllocateMesh(spherePrimitive.GetMeshPrimitive());
            break;
        }
        case BuiltinMeshType::CONE: {
            const auto conePrimitive =
                    ConePrimitive{1.0f, sceneConfig_.primitiveStackCount, sceneConfig_.primitiveSectorCount};
            meshGpu = AllocateMesh(conePrimitive.GetMeshPrimitive());
            break;
        }
        case BuiltinMeshType::CYLINDER: {
            const auto cylinderPrimitive =
                    CylinderPrimitive{1.0f, sceneConfig_.primitiveStackCount, sceneConfig_.primitiveSectorCount};
            meshGpu = AllocateMesh(cylinderPrimitive.GetMeshPrimitive());
            break;
        }
        case BuiltinMeshType::PLANE: {
            const auto planePrimitive = PlanePrimitive{1.0f};
            meshGpu = AllocateMesh(planePrimitive.GetMeshPrimitive());
            break;
        }
    }

    return meshGpu;
}

void SceneGpuBufferStorage::UpdateTransform(const std::uint32_t objectId, const TransformGpu& transformGpuData) const
{
    const auto offset = objectId * sizeof(TransformGpu);
    resourceManager_.SetBuffer(kTransformStorageBufferName, &transformGpuData, sizeof(TransformGpu), offset, false);
}

void SceneGpuBufferStorage::UpdateMaterial(const std::uint32_t objectId, const std::vector<uint8_t>& materialData) const
{
    const auto offset = objectId * materialData.size();
    resourceManager_.SetBuffer(kMaterialStorageBufferName, materialData.data(), materialData.size(), offset, false);
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> SceneGpuBufferStorage::GetGeometryBuffer() const
{
    return resourceManager_.GetBuffer(kGeometryBufferName);
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> SceneGpuBufferStorage::GetTransformStorageBuffer() const
{
    return resourceManager_.GetBuffer(kTransformStorageBufferName);
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> SceneGpuBufferStorage::GetMaterialStorageBuffer() const
{
    return resourceManager_.GetBuffer(kMaterialStorageBufferName);
}

std::uint32_t SceneGpuBufferStorage::GetAttributeCount() const { return sceneConfig_.attributeLayout.size(); }

std::vector<VkVertexInputBindingDescription> SceneGpuBufferStorage::GetBindingDescriptions() const
{
    std::vector<VkVertexInputBindingDescription> result;
    for (uint32_t bindingIndex = 0; const auto& [attributeType, accessorType]: sceneConfig_.attributeLayout) {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = bindingIndex++;
        bindingDescription.stride = GetAccessorSize(accessorType);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        result.push_back(bindingDescription);
    }

    return result;
}

std::vector<VkVertexInputAttributeDescription> SceneGpuBufferStorage::GetAttributeDescriptions() const
{
    std::vector<VkVertexInputAttributeDescription> result;
    for (uint32_t bindingIndex = 0; const auto& [attributeType, accessorType]: sceneConfig_.attributeLayout) {
        VkVertexInputAttributeDescription attributeDescription = {};
        attributeDescription.binding = bindingIndex;
        attributeDescription.location = bindingIndex++;
        attributeDescription.format = ConvertAccessorTypeToFormat(accessorType);
        attributeDescription.offset = 0;
        result.push_back(attributeDescription);
    }

    return result;
}

std::vector<MaterialComponent> SceneGpuBufferStorage::GetEnabledMaterialComponents() const
{
    return sceneConfig_.enabledMaterialComponents;
}

MeshGpu SceneGpuBufferStorage::AllocateMeshGpuInternal(const MeshPrimitive& meshPrimitive)
{
    // Check cache first for avoid duplicate GPU allocations
    if (meshCache_.contains(meshPrimitive.name)) {
        return meshCache_[meshPrimitive.name];
    }

    MeshGpu meshGpu;
    for (const auto& [attributeType, accessorType]: sceneConfig_.attributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = GetAccessorSize(accessorType);

        const auto accessor = meshPrimitive.attributes.at(attributeType);
        const size_t start = accessor.bufferView.byteOffset + accessor.byteOffset;
        const size_t length = accessor.count;
        const auto totalSize = length * accessorSize;
        globalBufferPos_ += totalSize;

        resourceManager_.SetBuffer(kGeometryBufferName, &accessor.bufferView.data[start], totalSize, offset, false);

        meshGpu.vertexOffsets.push_back(offset);
    }

    // Store indices after all vertex data
    const auto indicesAccessor = meshPrimitive.indices;
    const size_t start = indicesAccessor.bufferView.byteOffset + indicesAccessor.byteOffset;
    const size_t length = indicesAccessor.count;
    const auto totalSize = length * sizeof(std::uint16_t);

    meshGpu.indexOffset = globalBufferPos_;
    globalBufferPos_ += totalSize;
    resourceManager_.SetBuffer(kGeometryBufferName, &indicesAccessor.bufferView.data[start], totalSize,
                               meshGpu.indexOffset, false);
    meshGpu.indexCount = length;

    // Cache for future allocations of same mesh
    meshCache_[meshPrimitive.name] = meshGpu;
    return meshGpu;
}

} // namespace common::scene
