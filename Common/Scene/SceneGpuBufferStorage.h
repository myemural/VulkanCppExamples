/**
 * @file    SceneGpuBufferStorage.h
 * @brief   GPU buffer storage management for scene data.
 * @author  Mustafa Yemural (myemural)
 * @date    08.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "BuiltinPrimitives.h"
#include "CoreDefines.h"
#include "ResourceManager.h"
#include "SceneConfig.h"
#include "Transform.h"
#include "VulkanBuffer.h"

namespace common::scene
{

/**
 * @struct MeshGpu
 * @brief GPU representation of mesh data
 */
struct MeshGpu
{
    std::vector<VkDeviceSize> vertexOffsets;
    std::uint32_t indexOffset = UINT32_MAX;
    std::uint32_t indexCount = UINT32_MAX;
};

class COMMON_API SceneGpuBufferStorage
{
public:
    /**
     * @param resourceManager Resource manager reference.
     * @param sceneConfig Scene config.
     */
    SceneGpuBufferStorage(vulkan_framework::ResourceManager& resourceManager, const SceneConfig& sceneConfig);

    ~SceneGpuBufferStorage() = default;

    /**
     * @brief Allocates mesh or gets it from cache.
     * @param meshPrimitive The mesh primitive data accessor.
     * @return GPU representation with buffer offsets.
     */
    MeshGpu AllocateMesh(const MeshPrimitive& meshPrimitive);

    /**
     * @brief Allocate GPU memory for a builtin primitive mesh (cube, sphere, etc.) or gets it from cache.
     * @param builtinMeshType The primitive type to allocate
     * @return GPU representation with buffer offsets.
     */
    MeshGpu AllocateBuiltinMesh(const BuiltinMeshType& builtinMeshType);

    /**
     * @brief Update transform data for a scene object in GPU storage. Called when an object's transform changes.
     * @param objectId Unique object identifier.
     * @param transformGpuData Transform matrix data (model + normal matrix).
     */
    void UpdateTransform(std::uint32_t objectId, const TransformGpu& transformGpuData) const;

    /**
     * @brief Update material data for a scene object in GPU storage. Called when an object's material changes.
     * @param objectId Unique object identifier.
     * @param materialData Serialized material bytes (any type).
     */
    void UpdateMaterial(std::uint32_t objectId, const std::vector<uint8_t>& materialData) const;

    /**
     * @brief Get the geometry buffer (vertices, indices, UVs, normals, tangents).
     * @return Shared pointer to VulkanBuffer containing all mesh geometry.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetGeometryBuffer() const;

    /**
     * @brief Get the transform storage buffer.
     * @return Shared pointer to the transform storage buffer.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetTransformStorageBuffer() const;

    /**
     * @brief Get the material storage buffer.
     * @return Shared pointer to the material storage buffer.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetMaterialStorageBuffer() const;

    /**
     * @brief Get number of vertex attributes (position, normal, UV, etc.).
     * @return Count of vertex attributes in scene config.
     */
    [[nodiscard]] std::uint32_t GetAttributeCount() const;

    /**
     * @brief Get Vulkan vertex input binding descriptions.
     * @return Get Vulkan vertex input binding descriptions.
     */
    [[nodiscard]] std::vector<VkVertexInputBindingDescription> GetBindingDescriptions() const;

    /**
     * @brief Get Vulkan vertex input attribute descriptions.
     * @return Get Vulkan vertex input attribute descriptions.
     */
    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const;

    /**
     * @brief Get enabled material components for current scene's material system.
     * @return Get enabled material components for current scene's material system.
     */
    [[nodiscard]] std::vector<MaterialComponent> GetEnabledMaterialComponents() const;

private:
    MeshGpu AllocateMeshGpuInternal(const MeshPrimitive& meshPrimitive);

    static constexpr auto kGeometryBufferName = "SceneGeometryBuffer";
    static constexpr auto kTransformStorageBufferName = "SceneTransformStorageBuffer";
    static constexpr auto kMaterialStorageBufferName = "SceneMaterialStorageBuffer";
    static constexpr auto kBufferSizeInBytes = 16'000'000UL;

    vulkan_framework::ResourceManager& resourceManager_;
    SceneConfig sceneConfig_;

    std::uint32_t globalBufferPos_ = 0;
    std::unordered_map<std::string, MeshGpu> meshCache_;
};

} // namespace common::scene
