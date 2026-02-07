/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "Scene.h"

#include "ScenePrimitives.h"

namespace common::scene
{

Scene::Scene(vulkan_framework::ResourceManager& resourceManager, vulkan_framework::SceneConfig config)
    : resourceManager_{resourceManager}, sceneConfig_(std::move(config))
{
    vulkan_framework::ResourceDescriptor resourceCreateInfo;

    resourceCreateInfo.buffers = {
        {kSceneGeometryBufferName, kBufferSizeInBytes,
         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kSceneTransformStorageBufferName, kBufferSizeInBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kSceneMaterialStorageBufferName, kBufferSizeInBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    if (resourceCreateInfo.buffers.has_value()) {
        resourceManager_.CreateBuffers(resourceCreateInfo.buffers.value());
    }
}

std::shared_ptr<SceneObject> Scene::CreateObject(const std::string& objectName)
{
    auto object = std::make_shared<SceneObject>(*this, objectName);
    rootObjects_.push_back(object);
    return object;
}

void Scene::Traverse(const std::function<void(SceneObject&)>& func)
{
    for (auto& object: rootObjects_) {
        TraverseRecursive(*object, func);
    }
}

std::uint32_t Scene::GetAttributeCount() const { return sceneConfig_.attributeLayout.size(); }

std::vector<VkVertexInputBindingDescription> Scene::GetBindingDescriptions() const
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

std::vector<VkVertexInputAttributeDescription> Scene::GetAttributeDescriptions() const
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

std::shared_ptr<vulkan_wrapper::VulkanBuffer> Scene::GetGeometryBuffer() const
{
    return resourceManager_.GetBuffer(kSceneGeometryBufferName);
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> Scene::GetTransformStorageBuffer() const
{
    return resourceManager_.GetBuffer(kSceneTransformStorageBufferName);
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> Scene::GetMaterialStorageBuffer() const
{
    return resourceManager_.GetBuffer(kSceneMaterialStorageBufferName);
}

void Scene::TraverseRecursive(SceneObject& object, const std::function<void(SceneObject&)>& func)
{
    func(object);
    for (auto& childObject: object.GetChildren()) {
        TraverseRecursive(*childObject, func);
    }
}

MeshGpu Scene::AllocateMeshGpu(const utility::GltfMesh& mesh)
{
    if (meshCache_.contains(mesh.name)) {
        return meshCache_[mesh.name];
    }

    MeshGpu meshGpu;
    for (const auto& [attributeType, accessorType]: sceneConfig_.attributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = vulkan_framework::GetAccessorSize(accessorType);

        if (attributeType == vulkan_framework::AttributeType::POSITION) {
            const auto vertexPositons = mesh.attributes.positions;
            const auto vertexPositonsSize = vertexPositons.size() * accessorSize;
            globalBufferPos_ += vertexPositonsSize;

            resourceManager_.SetBuffer(kSceneGeometryBufferName, vertexPositons.data(), vertexPositonsSize, offset,
                                       false);
        } else if (attributeType == vulkan_framework::AttributeType::TEXCOORD) {
            const auto vertexUVs = mesh.attributes.texCoords0;
            const auto vertexUVsSize = vertexUVs.size() * accessorSize;
            globalBufferPos_ += vertexUVsSize;

            resourceManager_.SetBuffer(kSceneGeometryBufferName, vertexUVs.data(), vertexUVsSize, offset, false);
        } else if (attributeType == vulkan_framework::AttributeType::NORMAL) {
            const auto vertexNormals = mesh.attributes.normals;
            const auto vertexNormalsSize = vertexNormals.size() * accessorSize;
            globalBufferPos_ += vertexNormalsSize;

            resourceManager_.SetBuffer(kSceneGeometryBufferName, vertexNormals.data(), vertexNormalsSize, offset,
                                       false);
        } else if (attributeType == vulkan_framework::AttributeType::TANGENT) {
            const auto vertexTangents = mesh.attributes.tangents;
            const auto vertexTangentsSize = vertexTangents.size() * accessorSize;
            globalBufferPos_ += vertexTangentsSize;

            resourceManager_.SetBuffer(kSceneGeometryBufferName, vertexTangents.data(), vertexTangentsSize, offset,
                                       false);
        }

        meshGpu.vertexOffsets.push_back(offset);
    }

    const auto indices = mesh.indices;
    meshGpu.indexOffset = globalBufferPos_;
    const auto indicesSize = indices.size() * sizeof(std::uint16_t);
    globalBufferPos_ += indicesSize;
    resourceManager_.SetBuffer(kSceneGeometryBufferName, indices.data(), indicesSize, meshGpu.indexOffset, false);
    meshGpu.indexCount = indices.size();

    meshCache_[mesh.name] = meshGpu;
    return meshGpu;
}

MeshGpu Scene::AllocateBuiltinMeshGpu(const vulkan_framework::BuiltinMeshType& builtinMeshType)
{
    if (const auto meshName = GetBuiltinMeshName(builtinMeshType); meshCache_.contains(meshName)) {
        return meshCache_[meshName];
    }

    MeshGpu meshGpu;
    switch (builtinMeshType) {
        case vulkan_framework::BuiltinMeshType::CUBE:
            meshGpu = AllocateMeshGpu(vulkan_framework::CreateCubeMesh(1.0f));
            break;
        case vulkan_framework::BuiltinMeshType::SPHERE:
            meshGpu = AllocateMeshGpu(vulkan_framework::CreateSphereMesh(sceneConfig_.primitiveStackCount,
                                                                         sceneConfig_.primitiveSectorCount));
            break;
        case vulkan_framework::BuiltinMeshType::CONE:
            meshGpu = AllocateMeshGpu(vulkan_framework::CreateConeMesh(sceneConfig_.primitiveStackCount,
                                                                       sceneConfig_.primitiveSectorCount));
            break;
        case vulkan_framework::BuiltinMeshType::CYLINDER:
            meshGpu = AllocateMeshGpu(vulkan_framework::CreateCylinderMesh(sceneConfig_.primitiveStackCount,
                                                                           sceneConfig_.primitiveSectorCount));
            break;
        case vulkan_framework::BuiltinMeshType::PLANE:
            meshGpu = AllocateMeshGpu(vulkan_framework::CreatePlaneMesh(1.0f));
            break;
    }

    return meshGpu;
}

void Scene::UpdateSceneObjectTransformGpu(const std::uint32_t objectId, const TransformGpu& transformGpu) const
{
    const auto offset = objectId * sizeof(TransformGpu);
    resourceManager_.SetBuffer(kSceneTransformStorageBufferName, &transformGpu, sizeof(TransformGpu), offset,
                               false);
}

void Scene::UpdateSceneObjectMaterialGpu(const std::uint32_t objectId, const std::vector<uint8_t>& materialData) const
{
    const auto offset = objectId * materialData.size();
    resourceManager_.SetBuffer(kSceneMaterialStorageBufferName, materialData.data(), materialData.size(),
                               offset, false);
}

} // namespace common::scene
