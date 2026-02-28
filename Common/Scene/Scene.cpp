/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "Scene.h"

#include <algorithm>

#include "SceneGpuStorage.h"

namespace common::scene
{

Scene::Scene(vulkan_framework::ResourceManager& resourceManager, const SceneConfig& config)
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
void Scene::TraverseOrdered(const glm::vec3& cameraPosition, const TraverseFunc& func, const bool backToFront) const
{
    std::vector<std::shared_ptr<SceneObject>> objects;

    // Flatten hierarchy
    for (auto& root: rootObjects_) {
        CollectObjects(root, objects);
    }

    // Sort
    std::ranges::sort(objects, [&](const std::shared_ptr<SceneObject>& a, const std::shared_ptr<SceneObject>& b) {
        const glm::vec3 da = a->GetWorldPosition() - cameraPosition;
        const glm::vec3 db = b->GetWorldPosition() - cameraPosition;

        const float distA = glm::dot(da, da);
        const float distB = glm::dot(db, db);

        if (backToFront) {
            return distA > distB; // Far first
        }

        return distA < distB;     // Near first
    });

    // Traverse in sorted order
    for (auto& object: objects) {
        func(*object);
    }
}

std::shared_ptr<SceneObject> Scene::FindObjectByName(const std::string& name) const
{
    /// TODO: Object lookup will be added later.
    for (const auto& root: rootObjects_) {
        if (auto result = FindRecursive(root, name)) {
            return result;
        }
    }

    return nullptr;
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

std::shared_ptr<SceneObject> Scene::FindRecursive(const std::shared_ptr<SceneObject>& object, const std::string& name)
{
    if (object->GetName() == name) {
        return object;
    }

    for (const auto& child: object->GetChildren()) {
        if (auto result = FindRecursive(child, name)) {
            return result;
        }
    }

    return nullptr;
}

void Scene::CollectObjects(const std::shared_ptr<SceneObject>& object,
                           std::vector<std::shared_ptr<SceneObject>>& orderedObjects)
{
    orderedObjects.push_back(object);

    for (auto& child: object->GetChildren()) {
        CollectObjects(child, orderedObjects);
    }
}

std::uint32_t Scene::GenerateObjectId() { return currentObjectId_++; }

} // namespace common::scene
