/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "SceneGpuStorage.h"

#include <utility>

namespace common::scene
{

SceneGpuStorage::SceneGpuStorage(vulkan_framework::ResourceManager& resourceManager, const SceneConfig& sceneConfig)
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

MeshGpu SceneGpuStorage::AllocateMesh(const utility::GltfMesh& mesh) { return AllocateMeshGpuInternal(mesh); }

MeshGpu SceneGpuStorage::AllocateBuiltinMesh(const vulkan_framework::BuiltinMeshType& builtinMeshType)
{
    if (const auto meshName = vulkan_framework::GetBuiltinMeshName(builtinMeshType); meshCache_.contains(meshName)) {
        return meshCache_[meshName];
    }

    MeshGpu meshGpu;
    switch (builtinMeshType) {
        case vulkan_framework::BuiltinMeshType::CUBE:
            meshGpu = AllocateMeshGpuInternal(vulkan_framework::CreateCubeMesh(1.0f));
            break;
        case vulkan_framework::BuiltinMeshType::SPHERE:
            meshGpu = AllocateMeshGpuInternal(vulkan_framework::CreateSphereMesh(sceneConfig_.primitiveStackCount,
                                                                                 sceneConfig_.primitiveSectorCount));
            break;
        case vulkan_framework::BuiltinMeshType::CONE:
            meshGpu = AllocateMeshGpuInternal(vulkan_framework::CreateConeMesh(sceneConfig_.primitiveStackCount,
                                                                               sceneConfig_.primitiveSectorCount));
            break;
        case vulkan_framework::BuiltinMeshType::CYLINDER:
            meshGpu = AllocateMeshGpuInternal(vulkan_framework::CreateCylinderMesh(sceneConfig_.primitiveStackCount,
                                                                                   sceneConfig_.primitiveSectorCount));
            break;
        case vulkan_framework::BuiltinMeshType::PLANE:
            meshGpu = AllocateMeshGpuInternal(vulkan_framework::CreatePlaneMesh(1.0f));
            break;
    }

    return meshGpu;
}

void SceneGpuStorage::UpdateTransform(const std::uint32_t objectId, const TransformGpu& transformGpuData) const
{
    const auto offset = objectId * sizeof(TransformGpu);
    resourceManager_.SetBuffer(kTransformStorageBufferName, &transformGpuData, sizeof(TransformGpu), offset, false);
}

void SceneGpuStorage::UpdateMaterial(const std::uint32_t objectId, const std::vector<uint8_t>& materialData) const
{
    const auto offset = objectId * materialData.size();
    resourceManager_.SetBuffer(kMaterialStorageBufferName, materialData.data(), materialData.size(), offset, false);
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> SceneGpuStorage::GetGeometryBuffer() const
{
    return resourceManager_.GetBuffer(kGeometryBufferName);
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> SceneGpuStorage::GetTransformStorageBuffer() const
{
    return resourceManager_.GetBuffer(kTransformStorageBufferName);
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> SceneGpuStorage::GetMaterialStorageBuffer() const
{
    return resourceManager_.GetBuffer(kMaterialStorageBufferName);
}

std::uint32_t SceneGpuStorage::GetAttributeCount() const { return sceneConfig_.attributeLayout.size(); }

std::vector<VkVertexInputBindingDescription> SceneGpuStorage::GetBindingDescriptions() const
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

std::vector<VkVertexInputAttributeDescription> SceneGpuStorage::GetAttributeDescriptions() const
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

MeshGpu SceneGpuStorage::AllocateMeshGpuInternal(const utility::GltfMesh& mesh)
{
    // Check cache first for avoid duplicate GPU allocations
    if (meshCache_.contains(mesh.name)) {
        return meshCache_[mesh.name];
    }

    MeshGpu meshGpu;
    for (const auto& [attributeType, accessorType]: sceneConfig_.attributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = GetAccessorSize(accessorType);

        if (attributeType == AttributeType::POSITION) {
            const auto vertexPositons = mesh.attributes.positions;
            const auto vertexPositonsSize = vertexPositons.size() * accessorSize;
            globalBufferPos_ += vertexPositonsSize;

            resourceManager_.SetBuffer(kGeometryBufferName, vertexPositons.data(), vertexPositonsSize, offset, false);
        } else if (attributeType == AttributeType::TEXCOORD) {
            const auto vertexUVs = mesh.attributes.texCoords0;
            const auto vertexUVsSize = vertexUVs.size() * accessorSize;
            globalBufferPos_ += vertexUVsSize;

            resourceManager_.SetBuffer(kGeometryBufferName, vertexUVs.data(), vertexUVsSize, offset, false);
        } else if (attributeType == AttributeType::NORMAL) {
            const auto vertexNormals = mesh.attributes.normals;
            const auto vertexNormalsSize = vertexNormals.size() * accessorSize;
            globalBufferPos_ += vertexNormalsSize;

            resourceManager_.SetBuffer(kGeometryBufferName, vertexNormals.data(), vertexNormalsSize, offset, false);
        } else if (attributeType == AttributeType::TANGENT) {
            const auto vertexTangents = mesh.attributes.tangents;
            const auto vertexTangentsSize = vertexTangents.size() * accessorSize;
            globalBufferPos_ += vertexTangentsSize;

            resourceManager_.SetBuffer(kGeometryBufferName, vertexTangents.data(), vertexTangentsSize, offset, false);
        }

        meshGpu.vertexOffsets.push_back(offset);
    }

    // Store indices after all vertex data
    const auto indices = mesh.indices;
    meshGpu.indexOffset = globalBufferPos_;
    const auto indicesSize = indices.size() * sizeof(std::uint16_t);
    globalBufferPos_ += indicesSize;
    resourceManager_.SetBuffer(kGeometryBufferName, indices.data(), indicesSize, meshGpu.indexOffset, false);
    meshGpu.indexCount = indices.size();

    // Cache for future allocations of same mesh
    meshCache_[mesh.name] = meshGpu;
    return meshGpu;
}

} // namespace common::scene
