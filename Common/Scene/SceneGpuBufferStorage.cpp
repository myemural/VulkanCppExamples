/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "SceneGpuBufferStorage.h"

#include <utility>

namespace common::scene
{

namespace
{
    void GenerateTangents(const MeshPrimitive& mesh, std::vector<glm::vec4>& outTangents)
    {
        const auto& posAcc = mesh.attributes.at(AttributeType::POSITION);
        const auto& normalAcc = mesh.attributes.at(AttributeType::NORMAL);
        const auto& uvAcc = mesh.attributes.at(AttributeType::TEXCOORD);
        const auto& indexAcc = mesh.indices;

        const auto* P = reinterpret_cast<const glm::vec3*>(
                &posAcc.bufferView.data[posAcc.bufferView.byteOffset + posAcc.byteOffset]);

        const auto* N = reinterpret_cast<const glm::vec3*>(
                &normalAcc.bufferView.data[normalAcc.bufferView.byteOffset + normalAcc.byteOffset]);

        const auto* UV = reinterpret_cast<const glm::vec2*>(
                &uvAcc.bufferView.data[uvAcc.bufferView.byteOffset + uvAcc.byteOffset]);

        const auto* I = reinterpret_cast<const uint16_t*>(
                &indexAcc.bufferView.data[indexAcc.bufferView.byteOffset + indexAcc.byteOffset]);

        const size_t vertexCount = posAcc.count;
        const size_t indexCount = indexAcc.count;

        std::vector tangents(vertexCount, glm::vec3(0));
        std::vector bitangents(vertexCount, glm::vec3(0));

        for (size_t i = 0; i < indexCount; i += 3) {
            uint32_t i0 = I[i], i1 = I[i + 1], i2 = I[i + 2];

            glm::vec3 e1 = P[i1] - P[i0];
            glm::vec3 e2 = P[i2] - P[i0];

            glm::vec2 d1 = UV[i1] - UV[i0];
            glm::vec2 d2 = UV[i2] - UV[i0];

            float r = d1.x * d2.y - d2.x * d1.y;
            if (fabs(r) < 1e-8f) {
                continue;
            }

            r = 1.0f / r;

            glm::vec3 t = (e1 * d2.y - e2 * d1.y) * r;
            glm::vec3 b = (e2 * d1.x - e1 * d2.x) * r;

            tangents[i0] += t;
            tangents[i1] += t;
            tangents[i2] += t;
            bitangents[i0] += b;
            bitangents[i1] += b;
            bitangents[i2] += b;
        }

        // Fill result
        outTangents.resize(vertexCount);
        for (size_t i = 0; i < vertexCount; ++i) {
            const glm::vec3& n = N[i];
            glm::vec3 t = glm::normalize(tangents[i] - n * glm::dot(n, tangents[i]));
            float w = (glm::dot(glm::cross(n, t), bitangents[i]) < 0.0f) ? -1.0f : 1.0f;
            outTangents[i] = glm::vec4(t, w);
        }
    }
} // namespace

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
        case BuiltinMeshType::POINT: {
            const auto pointPrimitive = PointPrimitive{};
            meshGpu = AllocateMesh(pointPrimitive.GetMeshPrimitive());
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

        if (meshPrimitive.attributes.contains(attributeType)) {
            const auto accessor = meshPrimitive.attributes.at(attributeType);
            const size_t start = accessor.bufferView.byteOffset + accessor.byteOffset;
            const size_t length = accessor.count;
            const auto totalSize = length * accessorSize;
            globalBufferPos_ += totalSize;

            resourceManager_.SetBuffer(kGeometryBufferName, &accessor.bufferView.data[start], totalSize, offset, false);
        } else if (attributeType == AttributeType::TANGENT &&
                   !meshPrimitive.attributes.contains(AttributeType::TANGENT)) {
            // Generate tangents
            std::vector<glm::vec4> tangents;
            GenerateTangents(meshPrimitive, tangents);

            const auto totalSize = tangents.size() * accessorSize;
            globalBufferPos_ += totalSize;
            resourceManager_.SetBuffer(kGeometryBufferName, tangents.data(), totalSize, offset, false);
        } else {
            // Fallback for the missing attributes
            const auto posAccessor = meshPrimitive.attributes.at(AttributeType::POSITION);
            const auto totalSize = posAccessor.count * accessorSize;
            globalBufferPos_ += totalSize;
            std::vector<unsigned char> tempVector(totalSize, 0);
            resourceManager_.SetBuffer(kGeometryBufferName, tempVector.data(), totalSize, offset, false);
        }

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

    // Align global buffer position with 4
    globalBufferPos_ = (globalBufferPos_ + 3) & ~3;

    // Cache for future allocations of same mesh
    meshCache_[meshPrimitive.name] = meshGpu;
    return meshGpu;
}

} // namespace common::scene
