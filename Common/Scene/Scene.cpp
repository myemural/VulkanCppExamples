/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "Scene.h"

#include "SceneGpuStorage.h"

namespace common::scene
{

Scene::Scene(vulkan_framework::ResourceManager& resourceManager, const vulkan_framework::SceneConfig& config)
{
    gpuStorage_ = std::make_unique<SceneGpuStorage>(resourceManager, config);
}

std::shared_ptr<SceneObject> Scene::CreateObject(const std::string& objectName)
{
    auto object = std::make_shared<SceneObject>(*this, objectName, GenerateObjectId());
    rootObjects_.push_back(object);
    return object;
}

void Scene::Traverse(const TraverseFunc& func) const
{
    for (auto& object: rootObjects_) {
        TraverseRecursive(*object, func);
    }
}

std::uint32_t Scene::GetAttributeCount() const { return gpuStorage_->GetAttributeCount(); }

std::vector<VkVertexInputBindingDescription> Scene::GetBindingDescriptions() const
{
    return gpuStorage_->GetBindingDescriptions();
}

std::vector<VkVertexInputAttributeDescription> Scene::GetAttributeDescriptions() const
{
    return gpuStorage_->GetAttributeDescriptions();
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> Scene::GetGeometryBuffer() const
{
    return gpuStorage_->GetGeometryBuffer();
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> Scene::GetTransformStorageBuffer() const
{
    return gpuStorage_->GetTransformStorageBuffer();
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> Scene::GetMaterialStorageBuffer() const
{
    return gpuStorage_->GetMaterialStorageBuffer();
}

SceneGpuStorage& Scene::GetGpuStorage() const { return *gpuStorage_; }

void Scene::TraverseRecursive(const SceneObject& object, const TraverseFunc& func)
{
    func(object);
    for (auto& childObject: object.GetChildren()) {
        TraverseRecursive(*childObject, func);
    }
}

std::uint32_t Scene::GenerateObjectId() { return currentObjectId_++; }

} // namespace common::scene
