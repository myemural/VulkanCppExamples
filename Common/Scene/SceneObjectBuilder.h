/**
 * @file    SceneObjectBuilder.h
 * @brief   Contains scene object builder implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    07.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include "Scene.h"

namespace common::scene
{

class SceneObjectBuilder
{
public:
    COMMON_API explicit SceneObjectBuilder(Scene& scene, const std::string& name) : scene_{scene}
    {
        sceneObject_ = scene_.CreateObject(name);
    }

    COMMON_API SceneObjectBuilder& WithPosition(const glm::vec3& pos)
    {
        sceneObject_->SetPosition(pos);
        return *this;
    }

    COMMON_API SceneObjectBuilder& WithEulerAngles(const glm::vec3& eulerAngles)
    {
        sceneObject_->SetEulerAngles(eulerAngles);
        return *this;
    }

    COMMON_API SceneObjectBuilder& WithScale(const glm::vec3& scale)
    {
        sceneObject_->SetScale(scale);
        return *this;
    }

    COMMON_API SceneObjectBuilder& WithMesh(const utility::GltfMesh& mesh)
    {
        sceneObject_->SetMesh(mesh);
        return *this;
    }

    COMMON_API SceneObjectBuilder& WithBuiltinMesh(const vulkan_framework::BuiltinMeshType& builtinMeshType)
    {
        sceneObject_->SetBuiltinMesh(builtinMeshType);
        return *this;
    }

    template <typename MaterialType>
    SceneObjectBuilder& WithMaterial(const MaterialType& material)
    {
        sceneObject_->SetMaterial(material);
        return *this;
    }

    COMMON_API SceneObjectBuilder& AddChild(const SceneObjectBuilder& childBuilder)
    {
        childBuilders_.emplace_back(childBuilder);
        return *this;
    }

    [[nodiscard]] COMMON_API std::shared_ptr<SceneObject> Build()
    {
        for (auto& childBuilder : childBuilders_)
        {
            auto child = childBuilder.Build();
            sceneObject_->AddChild(child);
        }

        return sceneObject_;
    }

private:
    Scene& scene_;
    std::shared_ptr<SceneObject> sceneObject_;
    std::vector<SceneObjectBuilder> childBuilders_;
};

} // namespace common::scene
