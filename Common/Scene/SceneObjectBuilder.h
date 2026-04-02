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
    COMMON_API explicit SceneObjectBuilder(Scene& scene, const std::string& name);

    COMMON_API SceneObjectBuilder& WithTag(const std::string& tag);

    COMMON_API SceneObjectBuilder& WithPosition(const glm::vec3& pos);

    COMMON_API SceneObjectBuilder& WithEulerAngles(const glm::vec3& eulerAngles);

    COMMON_API SceneObjectBuilder& WithScale(const glm::vec3& scale);

    COMMON_API SceneObjectBuilder& WithMesh(const Mesh& mesh);

    COMMON_API SceneObjectBuilder& WithBuiltinMesh(const vulkan_framework::BuiltinMeshType& builtinMeshType);

    COMMON_API SceneObjectBuilder& WithMaterial(const Material& material);

    COMMON_API SceneObjectBuilder& AddChild(const SceneObjectBuilder& childBuilder);

    [[nodiscard]] COMMON_API std::shared_ptr<SceneObject> Build();

private:
    Scene& scene_;
    std::shared_ptr<SceneObject> sceneObject_;
    std::vector<SceneObjectBuilder> childBuilders_;
};

} // namespace common::scene
