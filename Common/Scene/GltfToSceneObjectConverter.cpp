/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "GltfToSceneObjectConverter.h"

#include <algorithm>
#include <utility>

#include <glm/gtc/type_ptr.hpp>

namespace common::scene
{

namespace
{
    /// TODO: These functions can be moved to a common place.
    ComponentType GetPrimitiveAccessorComponentType(const int componentType)
    {
        switch (componentType) {
            case TINYGLTF_COMPONENT_TYPE_BYTE:
                return ComponentType::SIGNED_BYTE;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                return ComponentType::UNSIGNED_BYTE;
            case TINYGLTF_COMPONENT_TYPE_SHORT:
                return ComponentType::SIGNED_SHORT;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                return ComponentType::UNSIGNED_SHORT;
            case TINYGLTF_COMPONENT_TYPE_INT:
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                return ComponentType::UNSIGNED_INT;
            case TINYGLTF_COMPONENT_TYPE_FLOAT:
            default:
                return ComponentType::FLOAT;
        }
    }

    DataType GetPrimitiveAccessorDataType(const int dataType)
    {
        switch (dataType) {
            case TINYGLTF_TYPE_SCALAR:
                return DataType::SCALAR;
            case TINYGLTF_TYPE_VEC2:
                return DataType::VEC2;
            case TINYGLTF_TYPE_VEC3:
                return DataType::VEC3;
            case TINYGLTF_TYPE_VEC4:
                return DataType::VEC4;
            case TINYGLTF_TYPE_MAT2:
                return DataType::MAT2;
            case TINYGLTF_TYPE_MAT3:
                return DataType::MAT3;
            case TINYGLTF_TYPE_MAT4:
                return DataType::MAT4;
            default:
                return DataType::VEC4;
        }
    }

    const std::unordered_map<std::string, AttributeType> kAttributeTypeMap = {{"POSITION", AttributeType::POSITION},
                                                                              {"NORMAL", AttributeType::NORMAL},
                                                                              {"TANGENT", AttributeType::TANGENT},
                                                                              {"TEXCOORD_0", AttributeType::TEXCOORD},
                                                                              {"COLOR_0", AttributeType::COLOR}};

    asset_manager::TextureAsset
    CreateTextureAsset(const tinygltf::Model& gltfModel, const int textureIndex, const std::string& basePath = "")
    {
        asset_manager::TextureAsset textureAsset;
        if (const auto& image = gltfModel.images[gltfModel.textures[textureIndex].source]; image.uri.empty()) {
            textureAsset.width = image.width;
            textureAsset.height = image.height;
            textureAsset.channels = image.component;
            textureAsset.data = image.image;
        } else {
            asset_manager::TextureLoader textureLoader{""};
            textureAsset =
                    *textureLoader.Load(basePath + image.uri); // TODO: This part will be fixed for external resources.
        }

        return textureAsset;
    }
} // namespace

GltfToSceneObjectConverter::GltfToSceneObjectConverter(Scene& scene, std::string defaultSamplerName)
    : scene_{scene}, defaultSamplerName_{std::move(defaultSamplerName)}
{
}

std::shared_ptr<SceneObject> GltfToSceneObjectConverter::Convert(const std::string& modelName,
                                                                 const asset_manager::GltfModelAsset& gltfModel)
{
    return ConvertInternal(modelName, gltfModel).Build();
}

SceneObjectBuilder GltfToSceneObjectConverter::ConvertToBuilder(const std::string& modelName,
                                                                const asset_manager::GltfModelAsset& gltfModel)
{
    return ConvertInternal(modelName, gltfModel);
}

SceneObjectBuilder GltfToSceneObjectConverter::ConvertInternal(const std::string& modelName,
                                                               const asset_manager::GltfModelAsset& gltfModel)
{
    const tinygltf::Model& model = gltfModel.GetModel();

    // Find current scene
    const auto currentSceneIndex = model.defaultScene > -1 ? model.defaultScene : 0;
    const auto& currentScene = model.scenes[currentSceneIndex];

    // Traverse nodes
    auto rootSceneObjectBuilder = SceneObjectBuilder(scene_, modelName);
    for (const auto nodeIndex: currentScene.nodes) {
        const auto& currentNode = model.nodes[nodeIndex];
        const auto& currentSceneObjectBuilder = CreateSceneObjectFromNode(model, model.nodes, currentNode);
        rootSceneObjectBuilder.AddChild(currentSceneObjectBuilder);
    }

    return rootSceneObjectBuilder;
}

SceneObjectBuilder GltfToSceneObjectConverter::CreateSceneObjectFromNode(const tinygltf::Model& gltfModel,
                                                                         const std::vector<tinygltf::Node>& nodes,
                                                                         const tinygltf::Node& node)
{
    auto sceneObjectBuilder = SceneObjectBuilder(scene_, node.name);

    // Process local transform
    /// TODO: Matrix processing will be done later.
    if (!node.translation.empty()) {
        sceneObjectBuilder.WithPosition(glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
    }
    if (!node.rotation.empty()) {
        sceneObjectBuilder.WithQuaternion(
                glm::vec4(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]));
    }
    if (!node.scale.empty()) {
        sceneObjectBuilder.WithScale(glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
    }

    // Process mesh and primitives
    if (const auto currentMeshIndex = node.mesh; currentMeshIndex >= 0) {
        int primitiveIndex = 0;
        for (const auto& currentPrimitive: gltfModel.meshes[currentMeshIndex].primitives) {
            const auto primitiveName =
                    gltfModel.meshes[currentMeshIndex].name + "_primitive_" + std::to_string(primitiveIndex++);
            const auto sceneObjectPrimitiveBuilder =
                    CreateSceneObjectFromPrimitive(gltfModel, primitiveName, currentPrimitive);
            sceneObjectBuilder.AddChild(sceneObjectPrimitiveBuilder);
        }
    }

    for (const auto& nodeIndex: node.children) {
        const auto& childNode = nodes[nodeIndex];
        const auto childSceneObjectBuilder = CreateSceneObjectFromNode(gltfModel, nodes, childNode);
        sceneObjectBuilder.AddChild(childSceneObjectBuilder);
    }

    return sceneObjectBuilder;
}

SceneObjectBuilder GltfToSceneObjectConverter::CreateSceneObjectFromPrimitive(const tinygltf::Model& gltfModel,
                                                                              const std::string& primitiveName,
                                                                              const tinygltf::Primitive& primitive)
{
    auto sceneObjectBuilder = SceneObjectBuilder(scene_, primitiveName);

    // Process mesh
    auto meshPrimitive = CreateMeshPrimitive(gltfModel, primitive);
    meshPrimitive.name = primitiveName;
    sceneObjectBuilder.WithMesh(meshPrimitive);

    // Process material
    if (primitive.material != -1) {
        const auto& currentGltfMaterial = gltfModel.materials[primitive.material];
        const auto material = CreateMaterial(gltfModel, currentGltfMaterial);
        sceneObjectBuilder.WithMaterial(material);
    }

    return sceneObjectBuilder;
}

MeshPrimitive GltfToSceneObjectConverter::CreateMeshPrimitive(const tinygltf::Model& gltfModel,
                                                              const tinygltf::Primitive& gltfPrimitive)
{
    MeshPrimitive result;

    // Process attributes
    for (const auto& [gltfAttribute, attributeType]: kAttributeTypeMap) {
        if (gltfPrimitive.attributes.contains(gltfAttribute)) {
            const auto& gltfAccessor = gltfModel.accessors[gltfPrimitive.attributes.at(gltfAttribute)];
            const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];
            const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];

            BufferView bufferView;
            bufferView.data = gltfBuffer.data.data();
            bufferView.bufferLength = gltfBuffer.data.size();
            bufferView.byteOffset = gltfBufferView.byteOffset;
            bufferView.byteLength = gltfBufferView.byteLength;
            bufferView.byteStride = gltfBufferView.byteStride;

            Accessor accessor;
            accessor.bufferView = bufferView;
            accessor.count = gltfAccessor.count;
            accessor.byteOffset = gltfAccessor.byteOffset;
            accessor.componentType = GetPrimitiveAccessorComponentType(gltfAccessor.componentType);
            accessor.type = GetPrimitiveAccessorDataType(gltfAccessor.type);

            result.attributes[attributeType] = accessor;
        }
    }

    // Process index buffer
    if (gltfPrimitive.indices >= 0) {
        const auto& gltfAccessor = gltfModel.accessors[gltfPrimitive.indices];
        const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];
        const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];

        BufferView bufferView;
        bufferView.data = gltfBuffer.data.data();
        bufferView.bufferLength = gltfBuffer.data.size();
        bufferView.byteOffset = gltfBufferView.byteOffset;
        bufferView.byteLength = gltfBufferView.byteLength;
        bufferView.byteStride = gltfBufferView.byteStride;

        Accessor accessor;
        accessor.bufferView = bufferView;
        accessor.count = gltfAccessor.count;
        accessor.byteOffset = gltfAccessor.byteOffset;
        accessor.componentType = GetPrimitiveAccessorComponentType(gltfAccessor.componentType);
        accessor.type = GetPrimitiveAccessorDataType(gltfAccessor.type);

        result.indices = accessor;
    }

    return result;
}

Material GltfToSceneObjectConverter::CreateMaterial(const tinygltf::Model& gltfModel,
                                                    const tinygltf::Material& gltfMaterial)
{
    Material result{};
    auto& sceneImageStorage = scene_.GetGpuImageStorage();
    const std::string materialName = gltfMaterial.name;

    // Check cache first
    if (materialCache_.contains(materialName)) {
        return materialCache_[materialName];
    }

    result.diffuseColor = glm::vec4(
            gltfMaterial.pbrMetallicRoughness.baseColorFactor[0], gltfMaterial.pbrMetallicRoughness.baseColorFactor[1],
            gltfMaterial.pbrMetallicRoughness.baseColorFactor[2], gltfMaterial.pbrMetallicRoughness.baseColorFactor[3]);
    result.albedoColor = result.diffuseColor;

    if (gltfMaterial.pbrMetallicRoughness.baseColorTexture.index >= 0) {
        const auto textureIndex = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
        const auto textureAsset = CreateTextureAsset(gltfModel, textureIndex);
        const auto textureId =
                sceneImageStorage.StoreTexture(materialName + "_diffuse", defaultSamplerName_, textureAsset);

        result.diffuseMap = textureId;
        result.albedoMap = textureId;
    }

    result.metallic = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
    result.roughness = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);

    if (gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
        const auto textureIndex = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
        const auto textureAsset = CreateTextureAsset(gltfModel, textureIndex);
        const auto textureId = sceneImageStorage.StoreTexture(materialName + "_metallicRoughness", defaultSamplerName_,
                                                              textureAsset, VK_FORMAT_R8G8B8A8_UNORM);

        result.metallicRoughnessMap = textureId;
    }

    if (gltfMaterial.normalTexture.index >= 0) {
        const auto textureIndex = gltfMaterial.normalTexture.index;
        const auto textureAsset = CreateTextureAsset(gltfModel, textureIndex);
        const auto textureId = sceneImageStorage.StoreTexture(materialName + "_normal", defaultSamplerName_,
                                                              textureAsset, VK_FORMAT_R8G8B8A8_UNORM);

        result.normalMap = textureId;
    }

    if (gltfMaterial.emissiveTexture.index >= 0) {
        const auto textureIndex = gltfMaterial.emissiveTexture.index;
        const auto textureAsset = CreateTextureAsset(gltfModel, textureIndex);
        const auto textureId =
                sceneImageStorage.StoreTexture(materialName + "_emissive", defaultSamplerName_, textureAsset);

        result.emissiveMap = textureId;
    }

    if (gltfMaterial.occlusionTexture.index >= 0) {
        const auto textureIndex = gltfMaterial.occlusionTexture.index;
        const auto textureAsset = CreateTextureAsset(gltfModel, textureIndex);
        const auto textureId = sceneImageStorage.StoreTexture(materialName + "_ao", defaultSamplerName_, textureAsset,
                                                              VK_FORMAT_R8G8B8A8_UNORM);

        result.ambientOcclusionMap = textureId;
    }

    // Specular Processing
    if (const auto it = gltfMaterial.extensions.find("KHR_materials_specular"); it != gltfMaterial.extensions.end()) {
        const tinygltf::Value& specularExt = it->second;

        if (specularExt.Has("specularFactor")) {
            result.specularStrength = static_cast<float>(specularExt.Get("specularFactor").GetNumberAsDouble()) * 0.5f;
        }

        if (specularExt.Has("specularColorFactor")) {
            const auto& specColorFactorArray = specularExt.Get("specularColorFactor").Get<tinygltf::Value::Array>();
            glm::vec3 specularColorFactor = {static_cast<float>(specColorFactorArray[0].GetNumberAsDouble()),
                                             static_cast<float>(specColorFactorArray[1].GetNumberAsDouble()),
                                             static_cast<float>(specColorFactorArray[2].GetNumberAsDouble())};

            // Normalize luminance and turn it to chromaticity (approximate to Disney BRDF specular tint)
            static constexpr glm::vec3 kLuminance(0.2126f, 0.7152f, 0.0722f);
            if (float luminance = glm::dot(specularColorFactor, kLuminance); luminance > 1e-5f) {
                glm::vec3 chromaticity = specularColorFactor / luminance;
                float deviation = glm::length(chromaticity - glm::vec3(1.0f));
                result.specularTint = glm::clamp(deviation / 1.414f, 0.0f, 1.0f);
            } else {
                result.specularTint = 0.0f;
            }
        }
    }

    // Clear Coat Processing
    if (const auto it = gltfMaterial.extensions.find("KHR_materials_clearcoat"); it != gltfMaterial.extensions.end()) {
        const tinygltf::Value& clearcoatExt = it->second;

        if (clearcoatExt.Has("clearcoatFactor")) {
            result.clearcoat = static_cast<float>(clearcoatExt.Get("clearcoatFactor").GetNumberAsDouble());
        }

        if (clearcoatExt.Has("clearcoatRoughnessFactor")) {
            result.clearcoatGloss =
                    1.0f - static_cast<float>(clearcoatExt.Get("clearcoatRoughnessFactor").GetNumberAsDouble());
        }
    }

    // Sheen Processing
    if (const auto it = gltfMaterial.extensions.find("KHR_materials_sheen"); it != gltfMaterial.extensions.end()) {
        if (const tinygltf::Value& sheenExt = it->second; sheenExt.Has("sheenColorFactor")) {
            glm::vec3 sheenColorFactor;
            sheenColorFactor.r = static_cast<float>(
                    sheenExt.Get("sheenColorFactor").Get<tinygltf::Value::Array>()[0].GetNumberAsDouble());
            sheenColorFactor.g = static_cast<float>(
                    sheenExt.Get("sheenColorFactor").Get<tinygltf::Value::Array>()[1].GetNumberAsDouble());
            sheenColorFactor.b = static_cast<float>(
                    sheenExt.Get("sheenColorFactor").Get<tinygltf::Value::Array>()[2].GetNumberAsDouble());

            // Sheen and sheen tint approximation (for Disney BRDF)
            float maxSheenColorFactor = std::max({sheenColorFactor.r, sheenColorFactor.g, sheenColorFactor.b});
            result.sheen = maxSheenColorFactor;

            glm::vec3 sheenColor = sheenColorFactor / maxSheenColorFactor;
            result.sheenTint = 1.0f - glm::dot(sheenColor, glm::vec3(1.0f / 3.0f));
        }
    }

    // Anisotropy Processing
    if (const auto it = gltfMaterial.extensions.find("KHR_materials_anisotropy"); it != gltfMaterial.extensions.end()) {

        if (const tinygltf::Value& anisotropyExt = it->second; anisotropyExt.Has("anisotropyStrength")) {
            result.anisotropic = static_cast<float>(anisotropyExt.Get("anisotropyStrength").GetNumberAsDouble());
        }
    }

    // Cache for future allocations of same material
    materialCache_[materialName] = result;
    return result;
}

} // namespace common::scene
