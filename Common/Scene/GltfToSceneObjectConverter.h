/**
 * @file    GltfToSceneObjectConverter.h
 * @brief   Contains converter class that converts glTF model to the SceneObject hierarchy.
 * @author  Mustafa Yemural (myemural)
 * @date    06.04.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "ModelAsset.h"
#include "Scene.h"

#include "SceneObjectBuilder.h"

namespace common::scene
{

class COMMON_API GltfToSceneObjectConverter
{
public:
    /**
     * @param scene Reference to the scene.
     * @param defaultSamplerName The name of the sampler to be used if no sampler information is found in the material.
     */
    GltfToSceneObjectConverter(Scene& scene, std::string defaultSamplerName);

    /**
     * @brief Creates a SceneObject from glTF model asset.
     * @param modelName Name of the model.
     * @param gltfModel glTF model asset.
     * @return Created SceneObject instance.
     */
    std::shared_ptr<SceneObject> Convert(const std::string& modelName, const asset_manager::GltfModelAsset& gltfModel);

    /**
     * @brief Creates a SceneObjectBuilder from glTF model asset.
     * @param modelName Name of the model.
     * @param gltfModel glTF model asset.
     * @return Created SceneObjectBuilder instance.
     */
    SceneObjectBuilder ConvertToBuilder(const std::string& modelName, const asset_manager::GltfModelAsset& gltfModel);

private:
    SceneObjectBuilder ConvertInternal(const std::string& modelName, const asset_manager::GltfModelAsset& gltfModel);

    SceneObjectBuilder CreateSceneObjectFromNode(const tinygltf::Model& gltfModel,
                                                 const std::vector<tinygltf::Node>& nodes,
                                                 const tinygltf::Node& node);

    SceneObjectBuilder CreateSceneObjectFromPrimitive(const tinygltf::Model& gltfModel,
                                                      const std::string& primitiveName,
                                                      const tinygltf::Primitive& primitive);

    static MeshPrimitive CreateMeshPrimitive(const tinygltf::Model& gltfModel,
                                             const tinygltf::Primitive& gltfPrimitive);

    Material CreateMaterial(const tinygltf::Model& gltfModel, const tinygltf::Material& gltfMaterial);

    Scene& scene_;
    std::string defaultSamplerName_;
    std::unordered_map<std::string, Material> materialCache_;
};

} // namespace common::scene
