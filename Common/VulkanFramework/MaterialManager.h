/**
 * @file    MaterialManager.h
 * @brief   Contains MaterialManager implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    10.01.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include "ResourceManager.h"
#include "SceneUtils.h"
#include "TextureAsset.h"

namespace common::vulkan_framework
{

class MaterialManager;

using SamplerId = uint32_t;
using MaterialId = uint32_t;

struct InternalTextureHandler
{
    TextureId textureId = -1;
    std::string textureName;
    std::string imageResourceName;
    std::string imageViewResourceName;
    std::string samplerResourceName;
};

class COMMON_API PhongMaterialBuilder
{
public:
    PhongMaterialBuilder(MaterialManager& materialManager, std::string materialName);

    PhongMaterialBuilder& SetDiffuseColor(const glm::vec3& diffuseColor);

    PhongMaterialBuilder& SetSpecularColor(const glm::vec3& specularColor);

    PhongMaterialBuilder& SetAmbientStrength(float ambientStrength);

    PhongMaterialBuilder& SetShininess(float shininess);

    PhongMaterialBuilder& SetSpecularStrength(float specularStrength);

    PhongMaterialBuilder& SetOpacity(float opacity);

    void Build();

private:
    MaterialManager& materialManager_;
    std::string materialName_;
    PhongMaterial phongMaterial_{};
};

class COMMON_API PhongTexturedMaterialBuilder
{
public:
    PhongTexturedMaterialBuilder(MaterialManager& materialManager, std::string materialName);

    PhongTexturedMaterialBuilder& SetDiffuseColor(const glm::vec3& diffuseColor);

    PhongTexturedMaterialBuilder& SetSpecularColor(const glm::vec3& specularColor);

    PhongTexturedMaterialBuilder& SetAmbientStrength(float ambientStrength);

    PhongTexturedMaterialBuilder& SetShininess(float shininess);

    PhongTexturedMaterialBuilder& SetSpecularStrength(float specularStrength);

    PhongTexturedMaterialBuilder& SetOpacity(float opacity);

    PhongTexturedMaterialBuilder& SetReflectivity(float reflectivity);

    PhongTexturedMaterialBuilder& SetDiffuseMap(const std::string& diffuseTextureName);

    PhongTexturedMaterialBuilder& SetSpecularMap(const std::string& specularTextureName);

    PhongTexturedMaterialBuilder& SetNormalMap(const std::string& normalTextureName);

    PhongTexturedMaterialBuilder& SetEmissiveMap(const std::string& emissiveTextureName);

    PhongTexturedMaterialBuilder& SetShininessMap(const std::string& shininessTextureName);

    PhongTexturedMaterialBuilder& SetOpacityMap(const std::string& opacityTextureName);

    PhongTexturedMaterialBuilder& SetAmbientOcclusionMap(const std::string& aoTextureName);

    PhongTexturedMaterialBuilder& SetHeightMap(const std::string& heightTextureName);

    void Build();

private:
    MaterialManager& materialManager_;
    std::string materialName_;
    PhongTexturedMaterial phongTexturedMaterial_{};
};

class COMMON_API MaterialManager
{
public:
    explicit MaterialManager(ResourceManager& resourceManager,
                             const std::shared_ptr<vulkan_wrapper::VulkanCommandPool>& cmdPool,
                             const std::shared_ptr<vulkan_wrapper::VulkanQueue>& queue);
    ~MaterialManager() = default;

    void LoadTexture(const std::string& textureName,
                     const std::string& samplerName,
                     const asset_manager::TextureAsset& textureAsset,
                     const VkFormat& format = VK_FORMAT_R8G8B8A8_SRGB,
                     bool mipmappingEnabled = false);

    void LoadCubemapTexture(const std::string& textureName,
                            const std::string& samplerName,
                            const asset_manager::TextureAsset& rightTextureAsset,
                            const asset_manager::TextureAsset& leftTextureAsset,
                            const asset_manager::TextureAsset& topTextureAsset,
                            const asset_manager::TextureAsset& bottomTextureAsset,
                            const asset_manager::TextureAsset& backTextureAsset,
                            const asset_manager::TextureAsset& frontTextureAsset,
                            const VkFormat& format = VK_FORMAT_R8G8B8A8_SRGB);

    TextureId GetTextureId(const std::string& textureName);

    [[nodiscard]] std::uint32_t GetTextureCount() const;

    [[nodiscard]] std::uint32_t GetCubemapTextureCount() const;

    [[nodiscard]] std::vector<VkDescriptorImageInfo> GetDescriptorImageInfos() const;

    [[nodiscard]] std::vector<VkDescriptorImageInfo> GetCubemapDescriptorImageInfo(const std::string& textureName);

    PhongMaterialBuilder CreatePhongMaterial(const std::string& materialName);

    PhongTexturedMaterialBuilder CreatePhongTexturedMaterial(const std::string& materialName);

    PhongMaterial& GetPhongMaterial(const std::string& materialName);

    PhongTexturedMaterial& GetPhongTexturedMaterial(const std::string& materialName);

    void RegisterMaterial(const std::string& materialName, const MaterialVariant& material);

private:
    static constexpr auto kVulkanImagePostfix = "_image";
    static constexpr auto kVulkanImageViewPostfix = "_imageView";

    ResourceManager& resourceManager_;
    std::shared_ptr<vulkan_wrapper::VulkanCommandPool> cmdPool_;
    std::shared_ptr<vulkan_wrapper::VulkanQueue> queue_;

    std::int32_t globalTextureId_ = 0;
    std::unordered_map<std::string, InternalTextureHandler> textureHandlers_;
    std::unordered_map<std::string, InternalTextureHandler> cubemapTextureHandlers_;
    std::unordered_map<std::string, MaterialVariant> materialHandlers_;
};

} // namespace common::vulkan_framework
