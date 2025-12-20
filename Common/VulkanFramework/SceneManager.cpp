/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "SceneManager.h"

#include <glm/glm.hpp>

#include "OrthographicCamera.h"
#include "PerspectiveCamera.h"
#include "ScenePrimitives.h"

namespace common::vulkan_framework
{

SceneManager::SceneManager(ResourceManager& resourceManager, const SceneConfig& sceneConfig)
    : resourceManager_{resourceManager}, sceneConfig_{sceneConfig}
{
    ResourceDescriptor resourceCreateInfo;

    resourceCreateInfo.Buffers = {{kMainBufferName, kBufferSizeInBytes,
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kStorageBufferName, kBufferSizeInBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    if (resourceCreateInfo.Buffers.has_value()) {
        resourceManager_.CreateBuffers(resourceCreateInfo.Buffers.value());
    }
}
std::uint32_t SceneManager::GetAttributeCount() const { return sceneConfig_.AttributeLayout.size(); }

std::vector<VkVertexInputBindingDescription> SceneManager::GetBindingDescriptions() const
{
    std::vector<VkVertexInputBindingDescription> result;
    for (uint32_t bindingIndex = 0; const auto& [attributeType, accessorType]: sceneConfig_.AttributeLayout) {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = bindingIndex++;
        bindingDescription.stride = GetAccessorSize(accessorType);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        result.push_back(bindingDescription);
    }

    return result;
}

std::vector<VkVertexInputAttributeDescription> SceneManager::GetAttributeDescriptions() const
{
    std::vector<VkVertexInputAttributeDescription> result;
    for (uint32_t bindingIndex = 0; const auto& [attributeType, accessorType]: sceneConfig_.AttributeLayout) {
        VkVertexInputAttributeDescription attributeDescription = {};
        attributeDescription.binding = bindingIndex;
        attributeDescription.location = bindingIndex++;
        attributeDescription.format = ConvertAccessorTypeToFormat(accessorType);
        attributeDescription.offset = 0;
        result.push_back(attributeDescription);
    }

    return result;
}

void SceneManager::AddCube(const std::string& objectName,
                           const glm::vec3& initialPosition,
                           const glm::vec3& initialRotation,
                           const glm::vec3& initialScale)
{
    MeshInfo meshInfo{};

    if (!isCubeBufferCreated_) {
        CreateCubeGeometry();
    }

    meshInfo.geometry = primitiveGeometries_[PrimitiveType::CUBE];
    meshInfo.transform = {initialPosition, initialRotation, initialScale};

    /// TODO: Will be increased
    if (sceneConfig_.MaterialSystem == MaterialSystem::PHONG) {
        meshInfo.material = PhongMaterial{};
    }

    meshInfo.objectId = currentObjectId_++;

    UpdateMeshDataGpu(meshInfo);

    meshes_[objectName] = meshInfo;
}

void SceneManager::AddSphere(const std::string& objectName,
                             const glm::vec3& initialPosition,
                             const glm::vec3& initialRotation,
                             const glm::vec3& initialScale)
{
    MeshInfo meshInfo{};

    if (!isSphereBufferCreated_) {
        CreateSphereGeometry();
    }

    meshInfo.geometry = primitiveGeometries_[PrimitiveType::SPHERE];
    meshInfo.transform = {initialPosition, initialRotation, initialScale};

    /// TODO: Will be increased
    if (sceneConfig_.MaterialSystem == MaterialSystem::PHONG) {
        meshInfo.material = PhongMaterial{};
    }

    meshInfo.objectId = currentObjectId_++;

    UpdateMeshDataGpu(meshInfo);

    meshes_[objectName] = meshInfo;
}

void SceneManager::AddCone(const std::string& objectName,
                           const glm::vec3& initialPosition,
                           const glm::vec3& initialRotation,
                           const glm::vec3& initialScale)
{
    MeshInfo meshInfo{};

    if (!isConeBufferCreated_) {
        CreateConeGeometry();
    }

    meshInfo.geometry = primitiveGeometries_[PrimitiveType::CONE];
    meshInfo.transform = {initialPosition, initialRotation, initialScale};

    /// TODO: Will be increased
    if (sceneConfig_.MaterialSystem == MaterialSystem::PHONG) {
        meshInfo.material = PhongMaterial{};
    }

    meshInfo.objectId = currentObjectId_++;

    UpdateMeshDataGpu(meshInfo);

    meshes_[objectName] = meshInfo;
}

void SceneManager::AddCylinder(const std::string& objectName,
                               const glm::vec3& initialPosition,
                               const glm::vec3& initialRotation,
                               const glm::vec3& initialScale)
{
    MeshInfo meshInfo{};

    if (!isCylinderBufferCreated_) {
        CreateCylinderGeometry();
    }

    meshInfo.geometry = primitiveGeometries_[PrimitiveType::CYLINDER];
    meshInfo.transform = {initialPosition, initialRotation, initialScale};

    /// TODO: Will be increased
    if (sceneConfig_.MaterialSystem == MaterialSystem::PHONG) {
        meshInfo.material = PhongMaterial{};
    }

    meshInfo.objectId = currentObjectId_++;

    UpdateMeshDataGpu(meshInfo);

    meshes_[objectName] = meshInfo;
}

void SceneManager::AddPlane(const std::string& objectName,
                            const glm::vec3& initialPosition,
                            const glm::vec3& initialRotation,
                            const glm::vec3& initialScale)
{
    MeshInfo meshInfo{};

    if (!isPlaneBufferCreated_) {
        CreatePlaneGeometry();
    }

    meshInfo.geometry = primitiveGeometries_[PrimitiveType::PLANE];
    meshInfo.transform = {initialPosition, initialRotation, initialScale};

    /// TODO: Will be increased
    if (sceneConfig_.MaterialSystem == MaterialSystem::PHONG) {
        meshInfo.material = PhongMaterial{};
    }

    meshInfo.objectId = currentObjectId_++;

    UpdateMeshDataGpu(meshInfo);

    meshes_[objectName] = meshInfo;
}

void SceneManager::MoveObject(const std::string& objectName, const glm::vec3& newPosition)
{
    meshes_[objectName].transform.translation = newPosition;
    UpdateMeshDataGpu(meshes_[objectName]);
}

void SceneManager::RotateObject(const std::string& objectName, const glm::vec3& newRotation)
{
    meshes_[objectName].transform.rotation = newRotation;
    UpdateMeshDataGpu(meshes_[objectName]);
}

void SceneManager::ScaleObject(const std::string& objectName, const glm::vec3& newScale)
{
    meshes_[objectName].transform.scale = newScale;
    UpdateMeshDataGpu(meshes_[objectName]);
}

void SceneManager::AddPerspectiveCamera(
        const std::string& cameraName, glm::vec3 position, float aspect, float fov, float nearPlane, float farPlane)
{
    cameras_[cameraName] = std::make_shared<utility::PerspectiveCamera>(position, aspect, fov, nearPlane, farPlane);

    if (activeCameraName_.empty()) {
        activeCameraName_ = cameraName;
    }
}

void SceneManager::AddOrthographicCamera(
        const std::string& cameraName, glm::vec3 position, float aspect, float size, float nearPlane, float farPlane)
{
    cameras_[cameraName] = std::make_shared<utility::OrthographicCamera>(position, aspect, size, nearPlane, farPlane);

    if (activeCameraName_.empty()) {
        activeCameraName_ = cameraName;
    }
}

void SceneManager::SetActiveCamera(const std::string& cameraName) { activeCameraName_ = cameraName; }

std::shared_ptr<utility::CameraBase> SceneManager::GetActiveCamera() { return cameras_[activeCameraName_]; }

glm::mat4 SceneManager::GetViewMatrix() { return cameras_[activeCameraName_]->GetViewMatrix(); }

glm::mat4 SceneManager::GetProjectionMatrix() { return cameras_[activeCameraName_]->GetProjectionMatrix(); }

MeshInfo& SceneManager::GetMesh(const std::string& objectName) { return meshes_[objectName]; }

std::unordered_map<std::string, MeshInfo>& SceneManager::GetAllMeshes() { return meshes_; }

std::shared_ptr<vulkan_wrapper::VulkanBuffer> SceneManager::GetGeometryBuffer() const
{
    return resourceManager_.GetBuffer(kMainBufferName);
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> SceneManager::GetStorageBuffer() const
{
    return resourceManager_.GetBuffer(kStorageBufferName);
}

void SceneManager::UpdateMeshDataGpu(const MeshInfo& meshInfo) const
{
    const MeshDataGpu meshData = meshInfo.GenerateMeshDataGpu();
    const auto offset = meshInfo.objectId * sizeof(meshData);
    resourceManager_.SetBuffer(kStorageBufferName, &meshData, sizeof(meshData), offset, false);
}

void SceneManager::CreateCubeGeometry()
{
    MeshInfo::GeometryInfo geometry;
    for (const auto& [attributeType, accessorType]: sceneConfig_.AttributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = GetAccessorSize(accessorType);

        if (attributeType == AttributeType::POSITION) {
            const auto vertexPositons = CreateCubePositions(1.0f);
            const auto vertexPositonsSize = vertexPositons.size() * accessorSize;
            globalBufferPos_ += vertexPositonsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexPositons.data(), vertexPositonsSize, offset, false);
        } else if (attributeType == AttributeType::TEXCOORD) {
            const auto vertexUVs = CreateCubeUVs();
            const auto vertexUVsSize = vertexUVs.size() * accessorSize;
            globalBufferPos_ += vertexUVsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexUVs.data(), vertexUVsSize, offset, false);
        } else if (attributeType == AttributeType::NORMAL) {
            const auto vertexNormals = CreateCubeNormals();
            const auto vertexNormalsSize = vertexNormals.size() * accessorSize;
            globalBufferPos_ += vertexNormalsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexNormals.data(), vertexNormalsSize, offset, false);
        }

        geometry.vertexOffsets.push_back(offset);
    }

    const auto indices = CreateCubeIndices();
    geometry.indexOffset = globalBufferPos_;
    const auto indicesSize = indices.size() * sizeof(std::uint16_t);
    globalBufferPos_ += indicesSize;
    resourceManager_.SetBuffer(kMainBufferName, indices.data(), indicesSize, geometry.indexOffset, false);
    geometry.indexCount = indices.size();

    primitiveGeometries_[PrimitiveType::CUBE] = geometry;
    isCubeBufferCreated_ = true;
}

void SceneManager::CreateSphereGeometry()
{
    MeshInfo::GeometryInfo geometry;
    for (const auto& [attributeType, accessorType]: sceneConfig_.AttributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = GetAccessorSize(accessorType);

        if (attributeType == AttributeType::POSITION) {
            const auto vertexPositons = CreateSpherePositions(1.0f, kPrimitiveStackCount, kPrimitiveSectorCount);
            const auto vertexPositonsSize = vertexPositons.size() * accessorSize;
            globalBufferPos_ += vertexPositonsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexPositons.data(), vertexPositonsSize, offset, false);
        } else if (attributeType == AttributeType::TEXCOORD) {
            const auto vertexUVs = CreateSphereUVs(kPrimitiveStackCount, kPrimitiveSectorCount);
            const auto vertexUVsSize = vertexUVs.size() * accessorSize;
            globalBufferPos_ += vertexUVsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexUVs.data(), vertexUVsSize, offset, false);
        } else if (attributeType == AttributeType::NORMAL) {
            const auto vertexNormals = CreateSphereNormals(kPrimitiveStackCount, kPrimitiveSectorCount);
            const auto vertexNormalsSize = vertexNormals.size() * accessorSize;
            globalBufferPos_ += vertexNormalsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexNormals.data(), vertexNormalsSize, offset, false);
        }

        geometry.vertexOffsets.push_back(offset);
    }

    const auto indices = CreateSphereIndices(kPrimitiveStackCount, kPrimitiveSectorCount);
    geometry.indexOffset = globalBufferPos_;
    const auto indicesSize = indices.size() * sizeof(std::uint16_t);
    globalBufferPos_ += indicesSize;
    resourceManager_.SetBuffer(kMainBufferName, indices.data(), indicesSize, geometry.indexOffset, false);
    geometry.indexCount = indices.size();

    primitiveGeometries_[PrimitiveType::SPHERE] = geometry;
    isSphereBufferCreated_ = true;
}

void SceneManager::CreateConeGeometry()
{
    MeshInfo::GeometryInfo geometry;
    for (const auto& [attributeType, accessorType]: sceneConfig_.AttributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = GetAccessorSize(accessorType);

        if (attributeType == AttributeType::POSITION) {
            const auto vertexPositons = CreateConePositions(1.0f, kPrimitiveStackCount, kPrimitiveSectorCount);
            const auto vertexPositonsSize = vertexPositons.size() * accessorSize;
            globalBufferPos_ += vertexPositonsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexPositons.data(), vertexPositonsSize, offset, false);
        } else if (attributeType == AttributeType::TEXCOORD) {
            const auto vertexUVs = CreateConeUVs(kPrimitiveStackCount, kPrimitiveSectorCount);
            const auto vertexUVsSize = vertexUVs.size() * accessorSize;
            globalBufferPos_ += vertexUVsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexUVs.data(), vertexUVsSize, offset, false);
        } else if (attributeType == AttributeType::NORMAL) {
            const auto vertexNormals = CreateConeNormals(kPrimitiveStackCount, kPrimitiveSectorCount);
            const auto vertexNormalsSize = vertexNormals.size() * accessorSize;
            globalBufferPos_ += vertexNormalsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexNormals.data(), vertexNormalsSize, offset, false);
        }

        geometry.vertexOffsets.push_back(offset);
    }

    const auto indices = CreateConeIndices(kPrimitiveStackCount, kPrimitiveSectorCount);
    geometry.indexOffset = globalBufferPos_;
    const auto indicesSize = indices.size() * sizeof(std::uint16_t);
    globalBufferPos_ += indicesSize;
    resourceManager_.SetBuffer(kMainBufferName, indices.data(), indicesSize, geometry.indexOffset, false);
    geometry.indexCount = indices.size();

    primitiveGeometries_[PrimitiveType::CONE] = geometry;
    isConeBufferCreated_ = true;
}

void SceneManager::CreateCylinderGeometry()
{
    MeshInfo::GeometryInfo geometry;
    for (const auto& [attributeType, accessorType]: sceneConfig_.AttributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = GetAccessorSize(accessorType);

        if (attributeType == AttributeType::POSITION) {
            const auto vertexPositons = CreateCylinderPositions(1.0f, kPrimitiveStackCount, kPrimitiveSectorCount);
            const auto vertexPositonsSize = vertexPositons.size() * accessorSize;
            globalBufferPos_ += vertexPositonsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexPositons.data(), vertexPositonsSize, offset, false);
        } else if (attributeType == AttributeType::TEXCOORD) {
            const auto vertexUVs = CreateCylinderUVs(kPrimitiveStackCount, kPrimitiveSectorCount);
            const auto vertexUVsSize = vertexUVs.size() * accessorSize;
            globalBufferPos_ += vertexUVsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexUVs.data(), vertexUVsSize, offset, false);
        } else if (attributeType == AttributeType::NORMAL) {
            const auto vertexNormals = CreateCylinderNormals(kPrimitiveStackCount, kPrimitiveSectorCount);
            const auto vertexNormalsSize = vertexNormals.size() * accessorSize;
            globalBufferPos_ += vertexNormalsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexNormals.data(), vertexNormalsSize, offset, false);
        }

        geometry.vertexOffsets.push_back(offset);
    }

    const auto indices = CreateCylinderIndices(kPrimitiveStackCount, kPrimitiveSectorCount);
    geometry.indexOffset = globalBufferPos_;
    const auto indicesSize = indices.size() * sizeof(std::uint16_t);
    globalBufferPos_ += indicesSize;
    resourceManager_.SetBuffer(kMainBufferName, indices.data(), indicesSize, geometry.indexOffset, false);
    geometry.indexCount = indices.size();

    primitiveGeometries_[PrimitiveType::CYLINDER] = geometry;
    isCylinderBufferCreated_ = true;
}

void SceneManager::CreatePlaneGeometry()
{
    MeshInfo::GeometryInfo geometry;
    for (const auto& [attributeType, accessorType]: sceneConfig_.AttributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = GetAccessorSize(accessorType);

        if (attributeType == AttributeType::POSITION) {
            const auto vertexPositons = CreatePlanePositions(2.0f);
            const auto vertexPositonsSize = vertexPositons.size() * accessorSize;
            globalBufferPos_ += vertexPositonsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexPositons.data(), vertexPositonsSize, offset, false);
        } else if (attributeType == AttributeType::TEXCOORD) {
            const auto vertexUVs = CreatePlaneUVs();
            const auto vertexUVsSize = vertexUVs.size() * accessorSize;
            globalBufferPos_ += vertexUVsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexUVs.data(), vertexUVsSize, offset, false);
        } else if (attributeType == AttributeType::NORMAL) {
            const auto vertexNormals = CreatePlaneNormals();
            const auto vertexNormalsSize = vertexNormals.size() * accessorSize;
            globalBufferPos_ += vertexNormalsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexNormals.data(), vertexNormalsSize, offset, false);
        }

        geometry.vertexOffsets.push_back(offset);
    }

    const auto indices = CreatePlaneIndices();
    geometry.indexOffset = globalBufferPos_;
    const auto indicesSize = indices.size() * sizeof(std::uint16_t);
    globalBufferPos_ += indicesSize;
    resourceManager_.SetBuffer(kMainBufferName, indices.data(), indicesSize, geometry.indexOffset, false);
    geometry.indexCount = indices.size();

    primitiveGeometries_[PrimitiveType::PLANE] = geometry;
    isPlaneBufferCreated_ = true;
}

} // namespace common::vulkan_framework
