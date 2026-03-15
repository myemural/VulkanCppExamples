/**
 * @file    SceneObject.h
 * @brief   Contains scene object implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    07.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "CoreDefines.h"
#include "Material.h"
#include "SceneGpuStorage.h"
#include "Transform.h"

namespace common::scene
{
class Scene;


class SceneObject : public std::enable_shared_from_this<SceneObject>
{
public:
    COMMON_API SceneObject(Scene& scene, std::string name, std::uint32_t objectId);

    COMMON_API void SetTag(const std::string& tag);

    COMMON_API void SetPosition(const glm::vec3& position);

    COMMON_API void SetEulerAngles(const glm::vec3& eulerAngles);

    COMMON_API void SetScale(const glm::vec3& scale);

    COMMON_API void SetMesh(const utility::GltfMesh& mesh);

    COMMON_API void SetBuiltinMesh(const vulkan_framework::BuiltinMeshType& builtinMeshType);

    COMMON_API glm::vec3 GetPosition() const;

    COMMON_API glm::vec3 GetWorldPosition();

    COMMON_API void SetMaterial(const Material& material);

    COMMON_API void SetParent(const std::shared_ptr<SceneObject>& parent);

    COMMON_API void AddChild(const std::shared_ptr<SceneObject>& childObject);

    [[nodiscard]] COMMON_API std::string GetName() const { return name_; }

    [[nodiscard]] COMMON_API std::uint32_t GetObjectId() const { return objectId_; }

    [[nodiscard]] COMMON_API std::string GetTag() const { return tag_; }

    [[nodiscard]] COMMON_API std::optional<MeshGpu> GetMeshGpu() const { return mesh_; }

    [[nodiscard]] COMMON_API const std::vector<std::shared_ptr<SceneObject>>& GetChildren() const { return children_; }

    [[nodiscard]] COMMON_API bool HasRenderable() const { return mesh_.has_value() && material_.has_value(); }

private:
    void RecalculateWorld();

    const glm::mat4& GetWorldMatrix();

    void MarkWorldDirty();

    void UpdateTransformGpu();

    COMMON_API void UpdateMaterialGpu(const std::vector<std::uint8_t>& materialData) const;

    Scene& scene_;

    std::string name_;
    std::uint32_t objectId_ = UINT32_MAX;
    std::string tag_;
    Transform transform_;
    std::optional<MeshGpu> mesh_ = std::nullopt;
    std::optional<Material> material_ = std::nullopt;

    std::shared_ptr<SceneObject> parent_ = nullptr;
    std::vector<std::shared_ptr<SceneObject>> children_;

    bool isWorldDirty_ = true;
    glm::mat4 worldMatrix_{1.0f};
};

} // namespace common::scene
