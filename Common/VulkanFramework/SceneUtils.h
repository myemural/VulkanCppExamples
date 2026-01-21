/**
 * @file    SceneUtils.h
 * @brief   Contains utility data structures and functions that used in SceneManager.
 * @author  Mustafa Yemural (myemural)
 * @date    11.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <cstdint>
#include <variant>

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

namespace common::vulkan_framework
{

enum class AttributeType
{
    POSITION,
    NORMAL,
    TANGENT,
    TEXCOORD,
    COLOR,
    JOINTS,
    WEIGHTS
};

enum class AccessorType
{
    VEC2,
    VEC3,
    VEC4
};

enum class PrimitiveType
{
    CUBE,
    SPHERE,
    CONE,
    CYLINDER,
    PLANE
};

using TextureId = std::int32_t;

struct COMMON_API MeshDataPhongGpu
{
    glm::mat4 model;
    glm::mat4 normalMatrix;
    glm::vec4 diffuseColor;
    glm::vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float opacity;
};

struct COMMON_API MeshDataPhongTexturedGpu
{
    glm::mat4 model;
    glm::mat4 normalMatrix;
    glm::vec4 diffuseColor;
    glm::vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float opacity;
    TextureId diffuseMap;
    TextureId specularMap;
    TextureId normalMap;
    TextureId emissiveMap;
    TextureId shininessMap;
    TextureId opacityMap;
    TextureId aoMap;
    TextureId heightMap;
};

struct COMMON_API MeshPushConstantsGpu
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec4 cameraPosition;
    std::uint32_t objectId;
};

struct COMMON_API PhongMaterial
{
    // Base colors
    glm::vec3 diffuseColor = glm::vec3(1.0f);  // RGB: albedo
    glm::vec3 specularColor = glm::vec3(1.0f); // RGB: specular reflectance color

    // Ambient params
    float ambientStrength = 0.1f; // Ambient strength factor

    // Specular params
    float shininess = 32.0f;       // Phong exponent
    float specularStrength = 1.0f; // Specular intensity

    // Optional tweaks
    float opacity = 1.0f; // For alpha blending
};

struct COMMON_API PhongTexturedMaterial
{
    // Base colors
    glm::vec3 diffuseColor = glm::vec3(1.0f);  // RGB: albedo
    glm::vec3 specularColor = glm::vec3(1.0f); // RGB: specular reflectance color

    // Ambient params
    float ambientStrength = 0.1f; // Ambient strength factor

    // Specular params
    float shininess = 32.0f;       // Phong exponent
    float specularStrength = 1.0f; // Specular intensity

    // Optional tweaks
    float opacity = 1.0f; // For alpha blending

    // Textures
    TextureId diffuseMap = -1;   // Diffuse map texture
    TextureId specularMap = -1;  // Specular map texture
    TextureId normalMap = -1;    // Normal map texture
    TextureId emissiveMap = -1;  // Emissive map texture
    TextureId shininessMap = -1; // Shininess map (reverse of the roughness map) texture
    TextureId opacityMap = -1;   // Opacity (alpha) map texture
    TextureId aoMap = -1;        // Ambient occlusion map texture
    TextureId heightMap = -1;    // Height map texture
};

using MaterialVariant = std::variant<PhongMaterial, PhongTexturedMaterial>;

struct COMMON_API MeshInfo
{
    struct GeometryInfo
    {
        std::vector<VkDeviceSize> vertexOffsets;
        std::uint32_t indexOffset = UINT32_MAX;
        std::uint32_t indexCount = UINT32_MAX;
    };

    struct TransformInfo
    {
        glm::vec3 translation = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);

        [[nodiscard]] glm::mat4 GetModelMatrix() const
        {
            auto transformMatrix = glm::mat4(1.0f);
            transformMatrix = glm::translate(transformMatrix, translation);
            transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            transformMatrix = glm::scale(transformMatrix, scale);

            return transformMatrix;
        }
    };

    GeometryInfo geometry;
    TransformInfo transform;
    MaterialVariant material;
    std::uint32_t objectId = UINT32_MAX;

    [[nodiscard]] std::variant<MeshDataPhongGpu, MeshDataPhongTexturedGpu> GenerateMeshDataGpu() const
    {
        std::variant<MeshDataPhongGpu, MeshDataPhongTexturedGpu> result;

        std::visit(
                [&](auto&& mat) {
                    using T = std::decay_t<decltype(mat)>;

                    glm::mat4 modelMatrix = transform.GetModelMatrix();
                    auto normalMatrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(modelMatrix))));

                    if constexpr (std::is_same_v<T, PhongMaterial>) {
                        // Phong material
                        MeshDataPhongGpu meshData{};
                        meshData.model = modelMatrix;
                        meshData.normalMatrix = normalMatrix;
                        meshData.diffuseColor = glm::vec4(mat.diffuseColor, 1.0f);
                        meshData.specularColor = glm::vec4(mat.specularColor, 1.0f);
                        meshData.ambientStrength = mat.ambientStrength;
                        meshData.shininess = mat.shininess;
                        meshData.specularStrength = mat.specularStrength;
                        meshData.opacity = mat.opacity;
                        result = meshData;
                    } else if (std::is_same_v<T, PhongTexturedMaterial>) {
                        // Phong material with textures
                        MeshDataPhongTexturedGpu meshData{};
                        meshData.model = modelMatrix;
                        meshData.normalMatrix = normalMatrix;
                        meshData.diffuseColor = glm::vec4(mat.diffuseColor, 1.0f);
                        meshData.specularColor = glm::vec4(mat.specularColor, 1.0f);
                        meshData.ambientStrength = mat.ambientStrength;
                        meshData.shininess = mat.shininess;
                        meshData.specularStrength = mat.specularStrength;
                        meshData.opacity = mat.opacity;
                        meshData.diffuseMap = mat.diffuseMap;
                        meshData.specularMap = mat.specularMap;
                        meshData.normalMap = mat.normalMap;
                        meshData.emissiveMap = mat.emissiveMap;
                        meshData.shininessMap = mat.shininessMap;
                        meshData.opacityMap = mat.opacityMap;
                        meshData.aoMap = mat.aoMap;
                        meshData.heightMap = mat.heightMap;
                        result = meshData;
                    }
                },
                material);
        return result;
    }

    [[nodiscard]] MeshPushConstantsGpu GenerateMeshPushConstantsGpu(const glm::mat4& viewMatrix,
                                                                    const glm::mat4& projMatrix,
                                                                    const glm::vec4& cameraPosition) const
    {
        MeshPushConstantsGpu pushConstants{};
        pushConstants.view = viewMatrix;
        pushConstants.projection = projMatrix;
        pushConstants.objectId = objectId;
        pushConstants.cameraPosition = cameraPosition;
        return pushConstants;
    }
};

inline std::uint32_t GetAccessorSize(const AccessorType accessorType)
{
    switch (accessorType) {
        case AccessorType::VEC2:
            return sizeof(glm::vec2);
        case AccessorType::VEC3:
            return sizeof(glm::vec3);
        case AccessorType::VEC4:
            return sizeof(glm::vec4);
    }
    return UINT32_MAX;
}

inline VkFormat ConvertAccessorTypeToFormat(const AccessorType accessorType)
{
    VkFormat format = VK_FORMAT_UNDEFINED;
    switch (accessorType) {
        case AccessorType::VEC2:
            format = VK_FORMAT_R32G32_SFLOAT;
            break;
        case AccessorType::VEC3:
            format = VK_FORMAT_R32G32B32_SFLOAT;
            break;
        case AccessorType::VEC4:
            format = VK_FORMAT_R32G32B32A32_SFLOAT;
            break;
    }
    return format;
}

} // namespace common::vulkan_framework
