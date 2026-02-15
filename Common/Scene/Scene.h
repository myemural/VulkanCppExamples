/**
 * @file    Scene.h
 * @brief   Contains scene hierarchy management.
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

class SceneGpuStorage;

using TraverseFunc = std::function<void(const SceneObject&)>;

class COMMON_API Scene
{
public:
    /**
     * @param resourceManager Reference to Vulkan resource manager.
     * @param config Scene configuration (attribute layout, primitive counts etc.).
     */
    Scene(vulkan_framework::ResourceManager& resourceManager, const vulkan_framework::SceneConfig& config);

    ~Scene() = default;

    /**
     * @brief Create a root-level scene object.
     * @param objectName Unique name for the object.
     * @return Shared pointer to newly created SceneObject.
     */
    std::shared_ptr<SceneObject> CreateObject(const std::string& objectName);


    /**
     * @brief Traverse (depth-first) entire scene tree and apply function to each object.
     * @param func Function/lambda to apply to each object.
     */
    void Traverse(const TraverseFunc& func) const;

    /**
     * Searches and returns scene object by name in the scene.
     * @param name Name of the scene object.
     * @return Returns related scene objects, otherwise nullptr.
     */
    [[nodiscard]] std::shared_ptr<SceneObject> FindObjectByName(const std::string& name) const;

    [[nodiscard]] std::uint32_t GetAttributeCount() const;

    [[nodiscard]] std::vector<VkVertexInputBindingDescription> GetBindingDescriptions() const;

    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const;

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetGeometryBuffer() const;

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetTransformStorageBuffer() const;

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetMaterialStorageBuffer() const;

    [[nodiscard]] SceneGpuStorage& GetGpuStorage() const;

private:
    static void TraverseRecursive(const SceneObject& object, const TraverseFunc& func);

    static std::shared_ptr<SceneObject> FindRecursive(const std::shared_ptr<SceneObject>& object, const std::string& name);

    std::uint32_t GenerateObjectId();

    std::uint32_t currentObjectId_ = 0;
    std::vector<std::shared_ptr<SceneObject>> rootObjects_;
    std::unique_ptr<SceneGpuStorage> gpuStorage_;
};

} // namespace common::scene
