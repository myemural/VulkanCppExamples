/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "SceneObject.h"

#include "Scene.h"

namespace common::scene
{

SceneObject::SceneObject(Scene& scene, std::string name, const std::uint32_t objectId)
    : scene_{scene}, name_{std::move(name)}, objectId_{objectId}
{
    UpdateTransformGpu();
}

void SceneObject::SetTag(const std::string& tag) { tag_ = tag; }

void SceneObject::SetPosition(const glm::vec3& position)
{
    transform_.SetPosition(position);
    MarkWorldDirty();
    UpdateTransformGpu();
}

void SceneObject::SetEulerAngles(const glm::vec3& eulerAngles)
{
    transform_.SetEulerAngles(eulerAngles);
    MarkWorldDirty();
    UpdateTransformGpu();
}

void SceneObject::SetScale(const glm::vec3& scale)
{
    transform_.SetScale(scale);
    MarkWorldDirty();
    UpdateTransformGpu();
}

void SceneObject::SetMesh(const utility::GltfMesh& mesh) { mesh_ = scene_.GetGpuStorage().AllocateMesh(mesh); }

void SceneObject::SetBuiltinMesh(const vulkan_framework::BuiltinMeshType& builtinMeshType)
{
    mesh_ = scene_.GetGpuStorage().AllocateBuiltinMesh(builtinMeshType);
}

void SceneObject::SetParent(const std::shared_ptr<SceneObject>& parent) { parent_ = parent; }

void SceneObject::AddChild(const std::shared_ptr<SceneObject>& childObject)
{
    childObject->SetParent(shared_from_this());
    children_.push_back(childObject);
    childObject->MarkWorldDirty();
}

void SceneObject::RecalculateWorld()
{
    const glm::mat4& local = transform_.GetLocalMatrix();

    if (parent_) {
        worldMatrix_ = parent_->GetWorldMatrix() * local;
    } else {
        worldMatrix_ = local;
    }

    transform_.ClearDirty();
    isWorldDirty_ = false;
}

const glm::mat4& SceneObject::GetWorldMatrix()
{
    if (isWorldDirty_ || transform_.IsDirty()) {
        RecalculateWorld();
    }

    return worldMatrix_;
}

void SceneObject::MarkWorldDirty()
{
    isWorldDirty_ = true;
    for (const auto& child: children_) {
        child->MarkWorldDirty();
    }
}

void SceneObject::UpdateTransformGpu()
{
    TransformGpu transformGpu{};
    transformGpu.modelMatrix = GetWorldMatrix();
    transformGpu.normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(transformGpu.modelMatrix))));
    scene_.GetGpuStorage().UpdateTransform(objectId_, transformGpu);
    for (const auto& child: children_) {
        child->UpdateTransformGpu();
    }
}

void SceneObject::UpdateMaterialGpu(const std::vector<std::uint8_t>& materialData) const
{
    const auto dataSize = static_cast<std::uint32_t>(materialData.size());
    const std::uint32_t alignedSize = utility::Align16(dataSize);

    if (alignedSize == dataSize) {
        scene_.GetGpuStorage().UpdateMaterial(objectId_, materialData);
        return;
    }

    // Needs padding for alignment
    std::vector<std::uint8_t> paddedData(alignedSize, 0);
    std::ranges::copy(materialData, paddedData.begin());
    scene_.GetGpuStorage().UpdateMaterial(objectId_, paddedData);
}

} // namespace common::scene
