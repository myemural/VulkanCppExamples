/**
 * @file    Scene.h
 * @brief   Contains scene implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    07.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "CoreDefines.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "SceneObject.h"

namespace common::scene
{

class COMMON_API Scene
{
public:
    Scene(vulkan_framework::ResourceManager& resourceManager, vulkan_framework::SceneConfig config);

    ~Scene() = default;

    std::shared_ptr<SceneObject> CreateObject(const std::string& objectName);

    void Traverse(const std::function<void(SceneObject&)>& func);

    // Attributes and bindings getters
    [[nodiscard]] std::uint32_t GetAttributeCount() const;
    [[nodiscard]] std::vector<VkVertexInputBindingDescription> GetBindingDescriptions() const;
    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const;

    // Buffer getters
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetGeometryBuffer() const;
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetTransformStorageBuffer() const;
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetMaterialStorageBuffer() const;

private:
    std::uint32_t GenerateObjectId() { return currentObjectId_++; }

    void TraverseRecursive(SceneObject& object, const std::function<void(SceneObject&)>& func);

    MeshGpu AllocateMeshGpu(const utility::GltfMesh& mesh);

    MeshGpu AllocateBuiltinMeshGpu(const vulkan_framework::BuiltinMeshType& builtinMeshType);

    void UpdateSceneObjectTransformGpu(std::uint32_t objectId, const TransformGpu& transformGpu) const;

    void UpdateSceneObjectMaterialGpu(std::uint32_t objectId, const std::vector<uint8_t>& materialData) const;

    static constexpr auto kSceneGeometryBufferName = "SceneGeometryBuffer";
    static constexpr auto kSceneTransformStorageBufferName = "SceneTransformStorageBuffer";
    static constexpr auto kSceneMaterialStorageBufferName = "SceneMaterialStorageBuffer";
    static constexpr auto kBufferSizeInBytes = 1'000'000UL; // 16 MB

    vulkan_framework::ResourceManager& resourceManager_;
    vulkan_framework::SceneConfig sceneConfig_;
    std::uint32_t currentObjectId_ = 0;
    std::uint32_t globalBufferPos_ = 0;
    std::unordered_map<std::string, MeshGpu> meshCache_;

    std::vector<std::shared_ptr<SceneObject>> rootObjects_;

    friend class SceneObject;
};

} // namespace common::scene
