/**
 * @file    Material.h
 * @brief   Contains generic material data structure and its helpers.
 * @author  Mustafa Yemural (myemural)
 * @date    15.03.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "CoreDefines.h"
#include "MemoryUtils.h"

namespace common::scene
{

enum class MaterialComponent : std::uint8_t
{
    DIFFUSE_COLOR_VEC4 = 0,
    ALBEDO_COLOR_VEC4,
    SPECULAR_COLOR_VEC4,

    AMBIENT_STRENGTH_FLOAT,
    SHININESS_FLOAT,
    ROUGHNESS_FLOAT,
    METALLIC_FLOAT,
    SPECULAR_STRENGTH_FLOAT,
    REFLECTIVITY_FLOAT,
    OPACITY_FLOAT,
    UV_SCALE_FLOAT,

    DIFFUSE_MAP_TEXTURE,
    SPECULAR_MAP_TEXTURE,
    NORMAL_MAP_TEXTURE,
    HEIGHT_MAP_TEXTURE,
    ROUGHNESS_MAP_TEXTURE,
    OPACITY_MAP_TEXTURE,
    EMISSIVE_MAP_TEXTURE,
    AMBIENT_OCCLUSION_MAP_TEXTURE,

    FLIP_NORMALS_INT,
};

struct COMMON_API Material
{
    // VEC4 values
    glm::vec4 diffuseColor = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 albedoColor = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 specularColor = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};

    // Float values
    float ambientStrength = 0.05f;
    float shininess = 128.0f;
    float roughness = 0.5f;
    float metallic = 0.5f;
    float specularStrength = 0.7f;
    float reflectivity = 0.0f;
    float opacity = 1.0f;
    float uvScale = 1.0f;

    // Mapping values
    int diffuseMap = -1;
    int specularMap = -1;
    int normalMap = -1;
    int heightMap = -1;
    int roughnessMap = -1;
    int opacityMap = -1;
    int emissiveMap = -1;
    int ambientOcclusionMap = -1;

    // Flag values
    int flipNormals = 0; // 0: False, 1: True
};

inline std::vector<std::uint8_t> SerializeMaterial(const Material& material,
                                                   const std::vector<MaterialComponent>& enabledMaterialComponents)
{
    std::vector<std::uint8_t> buffer;

    for (const MaterialComponent comp: enabledMaterialComponents) {
        switch (comp) {
            case MaterialComponent::DIFFUSE_COLOR_VEC4:
                utility::AppendBytes(buffer, material.diffuseColor);
                break;

            case MaterialComponent::ALBEDO_COLOR_VEC4:
                utility::AppendBytes(buffer, material.albedoColor);
                break;

            case MaterialComponent::SPECULAR_COLOR_VEC4:
                utility::AppendBytes(buffer, material.specularColor);
                break;

            case MaterialComponent::AMBIENT_STRENGTH_FLOAT:
                utility::AppendBytes(buffer, material.ambientStrength);
                break;

            case MaterialComponent::SHININESS_FLOAT:
                utility::AppendBytes(buffer, material.shininess);

            case MaterialComponent::ROUGHNESS_FLOAT:
                utility::AppendBytes(buffer, material.roughness);
                break;

            case MaterialComponent::METALLIC_FLOAT:
                utility::AppendBytes(buffer, material.metallic);
                break;

            case MaterialComponent::SPECULAR_STRENGTH_FLOAT:
                utility::AppendBytes(buffer, material.specularStrength);
                break;

            case MaterialComponent::REFLECTIVITY_FLOAT:
                utility::AppendBytes(buffer, material.reflectivity);
                break;

            case MaterialComponent::OPACITY_FLOAT:
                utility::AppendBytes(buffer, material.opacity);
                break;

            case MaterialComponent::UV_SCALE_FLOAT:
                utility::AppendBytes(buffer, material.uvScale);
                break;

            case MaterialComponent::DIFFUSE_MAP_TEXTURE:
                utility::AppendBytes(buffer, material.diffuseMap);
                break;

            case MaterialComponent::SPECULAR_MAP_TEXTURE:
                utility::AppendBytes(buffer, material.specularMap);
                break;

            case MaterialComponent::NORMAL_MAP_TEXTURE:
                utility::AppendBytes(buffer, material.normalMap);
                break;

            case MaterialComponent::HEIGHT_MAP_TEXTURE:
                utility::AppendBytes(buffer, material.heightMap);
                break;

            case MaterialComponent::ROUGHNESS_MAP_TEXTURE:
                utility::AppendBytes(buffer, material.roughnessMap);
                break;

            case MaterialComponent::OPACITY_MAP_TEXTURE:
                utility::AppendBytes(buffer, material.opacityMap);
                break;

            case MaterialComponent::EMISSIVE_MAP_TEXTURE:
                utility::AppendBytes(buffer, material.emissiveMap);
                break;

            case MaterialComponent::AMBIENT_OCCLUSION_MAP_TEXTURE:
                utility::AppendBytes(buffer, material.ambientOcclusionMap);
                break;

            case MaterialComponent::FLIP_NORMALS_INT:
                utility::AppendBytes(buffer, material.flipNormals);
                break;
        }
    }

    // 16-byte alignment
    constexpr std::size_t alignment = 16UL;
    if (const std::size_t remainder = buffer.size() % alignment; remainder != 0UL) {
        const std::size_t padding = alignment - remainder;
        buffer.insert(buffer.end(), padding, 0UL);
    }

    return buffer;
}

} // namespace common::scene
