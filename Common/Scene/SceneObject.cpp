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

void SceneObject::SetTag(const std::string& tag)
{
    tag_ = tag;
    for (const auto& child: children_) {
        child->SetTag(tag);
    }
}

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

void SceneObject::SetQuaternion(const glm::vec4& quat)
{
    transform_.SetQuaternion(quat);
    MarkWorldDirty();
    UpdateTransformGpu();
}

void SceneObject::SetScale(const glm::vec3& scale)
{
    transform_.SetScale(scale);
    MarkWorldDirty();
    UpdateTransformGpu();
}

void SceneObject::SetMesh(const MeshPrimitive& meshPrimitive)
{
    mesh_ = scene_.GetGpuBufferStorage().AllocateMesh(meshPrimitive);
}

void SceneObject::SetBuiltinMesh(const BuiltinMeshType& builtinMeshType)
{
    mesh_ = scene_.GetGpuBufferStorage().AllocateBuiltinMesh(builtinMeshType);
}

void SceneObject::SetMaterial(const Material& material)
{
    material_ = material;
    const auto materialData = SerializeMaterial(material, scene_.GetEnabledMaterialComponents());
    scene_.GetGpuBufferStorage().UpdateMaterial(objectId_, materialData);
}

void SceneObject::SetParent(const std::shared_ptr<SceneObject>& parent) { parent_ = parent; }

void SceneObject::AddChild(const std::shared_ptr<SceneObject>& childObject)
{
    childObject->SetParent(shared_from_this());
    children_.push_back(childObject);
    childObject->MarkWorldDirty();
    UpdateTransformGpu();
}

glm::vec3 SceneObject::GetPosition() const { return transform_.GetPosition(); }

glm::vec3 SceneObject::GetWorldPosition()
{
    const auto& world = GetWorldMatrix();
    return glm::vec3{world[3]};
}

bool SceneObject::HasRenderable() const { return mesh_.has_value() && material_.has_value(); }

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
    scene_.GetGpuBufferStorage().UpdateTransform(objectId_, transformGpu);
    for (const auto& child: children_) {
        child->UpdateTransformGpu();
    }
}

} // namespace common::scene
