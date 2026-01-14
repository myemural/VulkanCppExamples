/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "SceneManager.h"

#include <glm/glm.hpp>

#include "ModelLoader.h"
#include "OrthographicCamera.h"
#include "PerspectiveCamera.h"
#include "ScenePrimitives.h"

namespace common::vulkan_framework
{

SceneManager::SceneManager(ResourceManager& resourceManager,
                           MaterialManager& materialManager,
                           const SceneConfig& sceneConfig)
    : resourceManager_{resourceManager}, materialManager_(materialManager), sceneConfig_{sceneConfig}
{
    ResourceDescriptor resourceCreateInfo;

    resourceCreateInfo.buffers = {{kMainBufferName, kBufferSizeInBytes,
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kStorageBufferName, kBufferSizeInBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    if (resourceCreateInfo.buffers.has_value()) {
        resourceManager_.CreateBuffers(resourceCreateInfo.buffers.value());
    }

    // Set primitive stack and sector counts
    currentPrimStackCount_ = sceneConfig_.primitiveStackCount;
    currentPrimSectorCount_ = sceneConfig_.primitiveSectorCount;

    modelBasePath_ = sceneConfig_.modelBasePath;
}
std::uint32_t SceneManager::GetAttributeCount() const { return sceneConfig_.attributeLayout.size(); }

std::vector<VkVertexInputBindingDescription> SceneManager::GetBindingDescriptions() const
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

std::vector<VkVertexInputAttributeDescription> SceneManager::GetAttributeDescriptions() const
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
    if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG) {
        meshInfo.material = PhongMaterial{};
    } else if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG_TEXTURED) {
        meshInfo.material = PhongTexturedMaterial{};
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
    if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG) {
        meshInfo.material = PhongMaterial{};
    } else if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG_TEXTURED) {
        meshInfo.material = PhongTexturedMaterial{};
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
    if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG) {
        meshInfo.material = PhongMaterial{};
    } else if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG_TEXTURED) {
        meshInfo.material = PhongTexturedMaterial{};
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
    if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG) {
        meshInfo.material = PhongMaterial{};
    } else if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG_TEXTURED) {
        meshInfo.material = PhongTexturedMaterial{};
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
    if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG) {
        meshInfo.material = PhongMaterial{};
    } else if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG_TEXTURED) {
        meshInfo.material = PhongTexturedMaterial{};
    }

    meshInfo.objectId = currentObjectId_++;

    UpdateMeshDataGpu(meshInfo);

    meshes_[objectName] = meshInfo;
}

void SceneManager::AddModel(const std::string& modelName,
                            const std::string& modelPath,
                            const std::string& defaultSamplerName,
                            const glm::vec3& initialPosition,
                            const glm::vec3& initialRotation,
                            const glm::vec3& initialScale)
{
    utility::ModelLoader modelLoader{modelBasePath_};

    const auto modelHandler = modelLoader.LoadBinaryGltfFromFile(modelPath);

    if (!modelHandler) {
        throw std::runtime_error("Failed to load model \"" + modelPath + "\"");
    }

    // Create materials
    for (const auto& material: modelHandler->materials) {
        static int i = 0;

        auto&& materialBuilder = materialManager_.CreatePhongTexturedMaterial(material.name);
        materialBuilder.SetDiffuseColor(material.pbrMetallicRoughness.baseColor);

        if (const auto diffuseTextureIndex = material.pbrMetallicRoughness.baseColorTextureIndex;
            diffuseTextureIndex != -1) {
            const std::string diffuseTextureName = std::string("Texture_diffuse_") + std::to_string(i++);
            materialManager_.LoadTexture(diffuseTextureName, defaultSamplerName,
                                         modelHandler->textures[diffuseTextureIndex]);
            materialBuilder.SetDiffuseMap(diffuseTextureName);
        }

        if (const auto normalTextureIndex = material.normalTextureInfo.index; normalTextureIndex != -1) {
            const std::string normalTextureName = std::string("Texture_normal_") + std::to_string(i++);
            materialManager_.LoadTexture(normalTextureName, defaultSamplerName,
                                         modelHandler->textures[normalTextureIndex], VK_FORMAT_R8G8B8A8_UNORM);
            materialBuilder.SetNormalMap(normalTextureName);
        }


        if (const auto emissiveTextureIndex = material.emissionTextureIndex; emissiveTextureIndex != -1) {
            const std::string emissiveTextureName = std::string("Texture_emissive_") + std::to_string(i++);
            materialManager_.LoadTexture(emissiveTextureName, defaultSamplerName,
                                         modelHandler->textures[emissiveTextureIndex], VK_FORMAT_R8G8B8A8_UNORM);
            materialBuilder.SetEmissiveMap(emissiveTextureName);
        }

        materialBuilder.Build();
    }

    for (const auto& node: modelHandler->nodes) {
        if (node.meshIndex == UINT32_MAX) {
            continue;
        }

        const auto mesh = modelHandler->meshes[node.meshIndex];

        MeshInfo meshInfo{};
        meshInfo.geometry = CreateMeshGeometry(mesh);
        /// TODO: Transform system is completely wrong here, will be adjusted later.
        meshInfo.transform = {initialPosition, initialRotation, initialScale};
        meshInfo.transform.SetModelMatrix(meshInfo.transform.GetModelMatrix() * node.worldTransform);

        /// TODO: Will be increased
        if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG) {
            throw std::runtime_error("Only textured materials have been supported for models!");
        } else if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG_TEXTURED) {
            const auto meshMatIndex = mesh.materialIndex;
            meshInfo.material = materialManager_.GetPhongTexturedMaterial(modelHandler->materials[meshMatIndex].name);
        }

        meshInfo.objectId = currentObjectId_++;

        UpdateMeshDataGpu(meshInfo);

        meshes_[mesh.name] = meshInfo;

        AddToGroup(modelName, {mesh.name});
    }
}

void SceneManager::AddToGroup(const std::string& groupName, const std::initializer_list<std::string>& objectNames)
{
    for (const auto& objectName: objectNames) {
        groups_[groupName].emplace_back(objectName);
    }
}

bool SceneManager::IsInGroup(const std::string& objectName, const std::string& groupName)
{
    if (!groups_.contains(groupName)) {
        return false;
    }

    const auto result = std::ranges::find_if(groups_[groupName], [&objectName](const std::string& objectInGroup) {
        return objectName == objectInGroup;
    });

    return result != groups_[groupName].end();
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

void SceneManager::SetMaterial(const std::string& objectName, const std::string& materialName)
{
    auto& meshInfo = meshes_[objectName];
    if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG) {
        meshInfo.material = materialManager_.GetPhongMaterial(materialName);
    } else if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG_TEXTURED) {
        meshInfo.material = materialManager_.GetPhongTexturedMaterial(materialName);
    }

    UpdateMeshDataGpu(meshInfo);
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
    const auto meshData = meshInfo.GenerateMeshDataGpu();

    if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG) {
        const auto offset = meshInfo.objectId * sizeof(MeshDataPhongGpu);
        resourceManager_.SetBuffer(kStorageBufferName, &std::get<MeshDataPhongGpu>(meshData), sizeof(MeshDataPhongGpu),
                                   offset, false);
    } else if (sceneConfig_.currentMaterialSystem == MaterialSystem::PHONG_TEXTURED) {
        const auto offset = meshInfo.objectId * sizeof(MeshDataPhongTexturedGpu);
        resourceManager_.SetBuffer(kStorageBufferName, &std::get<MeshDataPhongTexturedGpu>(meshData),
                                   sizeof(MeshDataPhongTexturedGpu), offset, false);
    }
}

void SceneManager::CreateCubeGeometry()
{
    MeshInfo::GeometryInfo geometry;
    for (const auto& [attributeType, accessorType]: sceneConfig_.attributeLayout) {
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
        } else if (attributeType == AttributeType::TANGENT) {
            const auto vertexTangents = CreateCubeTangents();
            const auto vertexTangentsSize = vertexTangents.size() * accessorSize;
            globalBufferPos_ += vertexTangentsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexTangents.data(), vertexTangentsSize, offset, false);
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
    for (const auto& [attributeType, accessorType]: sceneConfig_.attributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = GetAccessorSize(accessorType);

        if (attributeType == AttributeType::POSITION) {
            const auto vertexPositons = CreateSpherePositions(1.0f, currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexPositonsSize = vertexPositons.size() * accessorSize;
            globalBufferPos_ += vertexPositonsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexPositons.data(), vertexPositonsSize, offset, false);
        } else if (attributeType == AttributeType::TEXCOORD) {
            const auto vertexUVs = CreateSphereUVs(currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexUVsSize = vertexUVs.size() * accessorSize;
            globalBufferPos_ += vertexUVsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexUVs.data(), vertexUVsSize, offset, false);
        } else if (attributeType == AttributeType::NORMAL) {
            const auto vertexNormals = CreateSphereNormals(currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexNormalsSize = vertexNormals.size() * accessorSize;
            globalBufferPos_ += vertexNormalsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexNormals.data(), vertexNormalsSize, offset, false);
        } else if (attributeType == AttributeType::TANGENT) {
            const auto vertexTangents = CreateSphereTangents(currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexTangentsSize = vertexTangents.size() * accessorSize;
            globalBufferPos_ += vertexTangentsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexTangents.data(), vertexTangentsSize, offset, false);
        }

        geometry.vertexOffsets.push_back(offset);
    }

    const auto indices = CreateSphereIndices(currentPrimStackCount_, currentPrimSectorCount_);
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
    for (const auto& [attributeType, accessorType]: sceneConfig_.attributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = GetAccessorSize(accessorType);

        if (attributeType == AttributeType::POSITION) {
            const auto vertexPositons = CreateConePositions(1.0f, currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexPositonsSize = vertexPositons.size() * accessorSize;
            globalBufferPos_ += vertexPositonsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexPositons.data(), vertexPositonsSize, offset, false);
        } else if (attributeType == AttributeType::TEXCOORD) {
            const auto vertexUVs = CreateConeUVs(currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexUVsSize = vertexUVs.size() * accessorSize;
            globalBufferPos_ += vertexUVsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexUVs.data(), vertexUVsSize, offset, false);
        } else if (attributeType == AttributeType::NORMAL) {
            const auto vertexNormals = CreateConeNormals(currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexNormalsSize = vertexNormals.size() * accessorSize;
            globalBufferPos_ += vertexNormalsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexNormals.data(), vertexNormalsSize, offset, false);
        } else if (attributeType == AttributeType::TANGENT) {
            const auto vertexTangents = CreateConeTangents(currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexTangentsSize = vertexTangents.size() * accessorSize;
            globalBufferPos_ += vertexTangentsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexTangents.data(), vertexTangentsSize, offset, false);
        }

        geometry.vertexOffsets.push_back(offset);
    }

    const auto indices = CreateConeIndices(currentPrimStackCount_, currentPrimSectorCount_);
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
    for (const auto& [attributeType, accessorType]: sceneConfig_.attributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = GetAccessorSize(accessorType);

        if (attributeType == AttributeType::POSITION) {
            const auto vertexPositons = CreateCylinderPositions(1.0f, currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexPositonsSize = vertexPositons.size() * accessorSize;
            globalBufferPos_ += vertexPositonsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexPositons.data(), vertexPositonsSize, offset, false);
        } else if (attributeType == AttributeType::TEXCOORD) {
            const auto vertexUVs = CreateCylinderUVs(currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexUVsSize = vertexUVs.size() * accessorSize;
            globalBufferPos_ += vertexUVsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexUVs.data(), vertexUVsSize, offset, false);
        } else if (attributeType == AttributeType::NORMAL) {
            const auto vertexNormals = CreateCylinderNormals(currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexNormalsSize = vertexNormals.size() * accessorSize;
            globalBufferPos_ += vertexNormalsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexNormals.data(), vertexNormalsSize, offset, false);
        } else if (attributeType == AttributeType::TANGENT) {
            const auto vertexTangents = CreateCylinderTangents(currentPrimStackCount_, currentPrimSectorCount_);
            const auto vertexTangentsSize = vertexTangents.size() * accessorSize;
            globalBufferPos_ += vertexTangentsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexTangents.data(), vertexTangentsSize, offset, false);
        }

        geometry.vertexOffsets.push_back(offset);
    }

    const auto indices = CreateCylinderIndices(currentPrimStackCount_, currentPrimSectorCount_);
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
    for (const auto& [attributeType, accessorType]: sceneConfig_.attributeLayout) {
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
        } else if (attributeType == AttributeType::TANGENT) {
            const auto vertexTangents = CreatePlaneTangents();
            const auto vertexTangentsSize = vertexTangents.size() * accessorSize;
            globalBufferPos_ += vertexTangentsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexTangents.data(), vertexTangentsSize, offset, false);
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

MeshInfo::GeometryInfo SceneManager::CreateMeshGeometry(const utility::GltfMesh& mesh)
{
    MeshInfo::GeometryInfo geometry;
    for (const auto& [attributeType, accessorType]: sceneConfig_.attributeLayout) {
        const std::uint32_t offset = globalBufferPos_;
        const auto accessorSize = GetAccessorSize(accessorType);

        if (attributeType == AttributeType::POSITION) {
            const auto vertexPositons = mesh.attributes.positions;
            const auto vertexPositonsSize = vertexPositons.size() * accessorSize;
            globalBufferPos_ += vertexPositonsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexPositons.data(), vertexPositonsSize, offset, false);
        } else if (attributeType == AttributeType::TEXCOORD) {
            const auto vertexUVs = mesh.attributes.texCoords0;
            const auto vertexUVsSize = vertexUVs.size() * accessorSize;
            globalBufferPos_ += vertexUVsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexUVs.data(), vertexUVsSize, offset, false);
        } else if (attributeType == AttributeType::NORMAL) {
            const auto vertexNormals = mesh.attributes.normals;
            const auto vertexNormalsSize = vertexNormals.size() * accessorSize;
            globalBufferPos_ += vertexNormalsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexNormals.data(), vertexNormalsSize, offset, false);
        } else if (attributeType == AttributeType::TANGENT) {
            const auto vertexTangents = mesh.attributes.tangents;
            const auto vertexTangentsSize = vertexTangents.size() * accessorSize;
            globalBufferPos_ += vertexTangentsSize;

            resourceManager_.SetBuffer(kMainBufferName, vertexTangents.data(), vertexTangentsSize, offset, false);
        }

        geometry.vertexOffsets.push_back(offset);
    }

    const auto indices = mesh.indices;
    geometry.indexOffset = globalBufferPos_;
    const auto indicesSize = indices.size() * sizeof(std::uint16_t);
    globalBufferPos_ += indicesSize;
    resourceManager_.SetBuffer(kMainBufferName, indices.data(), indicesSize, geometry.indexOffset, false);
    geometry.indexCount = indices.size();

    return geometry;
}

} // namespace common::vulkan_framework
