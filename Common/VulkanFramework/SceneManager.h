/**
 * @file    SceneManager.h
 * @brief   Contains SceneManager implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    11.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <glm/vec3.hpp>

#include "CameraBase.h"
#include "ResourceManager.h"
#include "SceneUtils.h"

namespace common::vulkan_framework
{

enum class MaterialSystem
{
    PHONG,
    PHONG_TEXTURED,
    PBR
};

struct COMMON_API SceneConfig
{
    std::vector<std::pair<AttributeType, AccessorType>> AttributeLayout;
    MaterialSystem MaterialSystem = MaterialSystem::PHONG;
};

class COMMON_API SceneManager
{
public:
    explicit SceneManager(ResourceManager& resourceManager, const SceneConfig& sceneConfig);
    ~SceneManager() = default;

    // Attributes and bindings
    [[nodiscard]] std::uint32_t GetAttributeCount() const;
    [[nodiscard]] std::vector<VkVertexInputBindingDescription> GetBindingDescriptions() const;
    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const;

    // Primitives
    void AddCube(const std::string& objectName,
                 const glm::vec3& initialPosition = glm::vec3{0.0f},
                 const glm::vec3& initialRotation = glm::vec3{0.0f},
                 const glm::vec3& initialScale = glm::vec3{1.0f});

    void AddSphere(const std::string& objectName,
                   const glm::vec3& initialPosition = glm::vec3{0.0f},
                   const glm::vec3& initialRotation = glm::vec3{0.0f},
                   const glm::vec3& initialScale = glm::vec3{1.0f});

    void AddCone(const std::string& objectName,
                 const glm::vec3& initialPosition = glm::vec3{0.0f},
                 const glm::vec3& initialRotation = glm::vec3{0.0f},
                 const glm::vec3& initialScale = glm::vec3{1.0f});

    void AddCylinder(const std::string& objectName,
                     const glm::vec3& initialPosition = glm::vec3{0.0f},
                     const glm::vec3& initialRotation = glm::vec3{0.0f},
                     const glm::vec3& initialScale = glm::vec3{1.0f});

    void AddPlane(const std::string& objectName,
                  const glm::vec3& initialPosition = glm::vec3{0.0f},
                  const glm::vec3& initialRotation = glm::vec3{0.0f},
                  const glm::vec3& initialScale = glm::vec3{1.0f});

    // Change transform
    void MoveObject(const std::string& objectName, const glm::vec3& newPosition);
    void RotateObject(const std::string& objectName, const glm::vec3& newRotation);
    void ScaleObject(const std::string& objectName, const glm::vec3& newScale);

    // Change material
    template<typename MaterialType>
    void SetObjectMaterial(const std::string& objectName, const MaterialType& material)
    {
        auto& meshInfo = meshes_[objectName];
        std::get<MaterialType>(meshInfo.material) = material;

        UpdateMeshDataGpu(meshInfo);
    }

    // Cameras
    void AddPerspectiveCamera(const std::string& cameraName,
                              glm::vec3 position,
                              float aspect,
                              float fov = 45.0f,
                              float nearPlane = 0.1f,
                              float farPlane = 100.0f);

    void AddOrthographicCamera(const std::string& cameraName,
                               glm::vec3 position,
                               float aspect,
                               float size = 10.0f,
                               float nearPlane = 0.1f,
                               float farPlane = 100.0f);

    void SetActiveCamera(const std::string& cameraName);

    std::shared_ptr<utility::CameraBase> GetActiveCamera();
    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix();

    // Getters
    MeshInfo& GetMesh(const std::string& objectName);
    std::unordered_map<std::string, MeshInfo>& GetAllMeshes();
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetGeometryBuffer() const;
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetStorageBuffer() const;

private:
    void UpdateMeshDataGpu(const MeshInfo& meshInfo) const;

    void CreateCubeGeometry();
    void CreateSphereGeometry();
    void CreateConeGeometry();
    void CreateCylinderGeometry();
    void CreatePlaneGeometry();

    static constexpr auto kMainBufferName = "MainBuffer";
    static constexpr auto kStorageBufferName = "StorageBuffer";
    static constexpr auto kBufferSizeInBytes = 1'000'000UL; // 16 MB
    static constexpr auto kPrimitiveSectorCount = 24U;
    static constexpr auto kPrimitiveStackCount = 24U;

    ResourceManager& resourceManager_;
    SceneConfig sceneConfig_;

    std::uint32_t currentObjectId_ = 0;
    std::uint32_t globalBufferPos_ = 0;
    std::unordered_map<std::string, MeshInfo> meshes_;

    std::unordered_map<std::string, std::shared_ptr<utility::CameraBase>> cameras_;
    std::string activeCameraName_;

    // Preloading primitive buffers
    std::unordered_map<PrimitiveType, MeshInfo::GeometryInfo> primitiveGeometries_;
    bool isCubeBufferCreated_ = false;
    bool isSphereBufferCreated_ = false;
    bool isConeBufferCreated_ = false;
    bool isCylinderBufferCreated_ = false;
    bool isPlaneBufferCreated_ = false;
};

} // namespace common::vulkan_framework
