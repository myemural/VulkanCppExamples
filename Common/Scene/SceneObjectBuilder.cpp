/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "SceneObjectBuilder.h"

namespace common::scene
{

SceneObjectBuilder::SceneObjectBuilder(Scene& scene, const std::string& name) : scene_{scene}
{
    sceneObject_ = scene_.CreateObject(name);
}

SceneObjectBuilder& SceneObjectBuilder::WithTag(const std::string& tag)
{
    sceneObject_->SetTag(tag);
    for (auto& childBuilder: childBuilders_) {
        childBuilder.WithTag(tag);
    }
    return *this;
}

SceneObjectBuilder& SceneObjectBuilder::WithPosition(const glm::vec3& pos)
{
    sceneObject_->SetPosition(pos);
    return *this;
}

SceneObjectBuilder& SceneObjectBuilder::WithEulerAngles(const glm::vec3& eulerAngles)
{
    sceneObject_->SetEulerAngles(eulerAngles);
    return *this;
}

SceneObjectBuilder& SceneObjectBuilder::WithQuaternion(const glm::vec4& quat)
{
    sceneObject_->SetQuaternion(quat);
    return *this;
}

SceneObjectBuilder& SceneObjectBuilder::WithScale(const glm::vec3& scale)
{
    sceneObject_->SetScale(scale);
    return *this;
}

SceneObjectBuilder& SceneObjectBuilder::WithMesh(const MeshPrimitive& meshPrimitive)
{
    sceneObject_->SetMesh(meshPrimitive);
    return *this;
}

SceneObjectBuilder& SceneObjectBuilder::WithBuiltinMesh(const BuiltinMeshType& builtinMeshType)
{
    sceneObject_->SetBuiltinMesh(builtinMeshType);
    return *this;
}

SceneObjectBuilder& SceneObjectBuilder::WithMaterial(const Material& material)
{
    sceneObject_->SetMaterial(material);
    return *this;
}

SceneObjectBuilder& SceneObjectBuilder::AddChild(const SceneObjectBuilder& childBuilder)
{
    childBuilders_.emplace_back(childBuilder);
    return *this;
}

std::shared_ptr<SceneObject> SceneObjectBuilder::Build()
{
    for (auto& childBuilder: childBuilders_) {
        auto child = childBuilder.Build();
        sceneObject_->AddChild(child);
    }

    return sceneObject_;
}

} // namespace common::scene
