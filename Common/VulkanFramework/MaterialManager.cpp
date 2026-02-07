/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "MaterialManager.h"

#include <utility>

#include "MathUtils.h"
#include "VulkanHelpers.h"

namespace common::vulkan_framework
{

PhongMaterialBuilder::PhongMaterialBuilder(MaterialManager& materialManager, std::string materialName)
    : materialManager_(materialManager), materialName_(std::move(materialName))
{
}

PhongMaterialBuilder& PhongMaterialBuilder::SetDiffuseColor(const glm::vec3& diffuseColor)
{
    phongMaterial_.diffuseColor = diffuseColor;
    return *this;
}

PhongMaterialBuilder& PhongMaterialBuilder::SetSpecularColor(const glm::vec3& specularColor)
{
    phongMaterial_.specularColor = specularColor;
    return *this;
}

PhongMaterialBuilder& PhongMaterialBuilder::SetAmbientStrength(const float ambientStrength)
{
    phongMaterial_.ambientStrength = ambientStrength;
    return *this;
}

PhongMaterialBuilder& PhongMaterialBuilder::SetShininess(const float shininess)
{
    phongMaterial_.shininess = shininess;
    return *this;
}

PhongMaterialBuilder& PhongMaterialBuilder::SetSpecularStrength(const float specularStrength)
{
    phongMaterial_.specularStrength = specularStrength;
    return *this;
}

PhongMaterialBuilder& PhongMaterialBuilder::SetOpacity(const float opacity)
{
    phongMaterial_.opacity = opacity;
    return *this;
}

void PhongMaterialBuilder::Build() { materialManager_.RegisterMaterial(materialName_, phongMaterial_); }

PhongTexturedMaterialBuilder::PhongTexturedMaterialBuilder(MaterialManager& materialManager, std::string materialName)
    : materialManager_(materialManager), materialName_(std::move(materialName))
{
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetDiffuseColor(const glm::vec3& diffuseColor)
{
    phongTexturedMaterial_.diffuseColor = diffuseColor;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetSpecularColor(const glm::vec3& specularColor)
{
    phongTexturedMaterial_.specularColor = specularColor;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetAmbientStrength(const float ambientStrength)
{
    phongTexturedMaterial_.ambientStrength = ambientStrength;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetShininess(const float shininess)
{
    phongTexturedMaterial_.shininess = shininess;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetSpecularStrength(const float specularStrength)
{
    phongTexturedMaterial_.specularStrength = specularStrength;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetOpacity(const float opacity)
{
    phongTexturedMaterial_.opacity = opacity;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetReflectivity(float reflectivity)
{
    phongTexturedMaterial_.reflectivity = reflectivity;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetDiffuseMap(const std::string& diffuseTextureName)
{
    const auto textureId = materialManager_.GetTextureId(diffuseTextureName);
    phongTexturedMaterial_.diffuseMap = textureId;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetSpecularMap(const std::string& specularTextureName)
{
    const auto textureId = materialManager_.GetTextureId(specularTextureName);
    phongTexturedMaterial_.specularMap = textureId;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetNormalMap(const std::string& normalTextureName)
{
    const auto textureId = materialManager_.GetTextureId(normalTextureName);
    phongTexturedMaterial_.normalMap = textureId;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetEmissiveMap(const std::string& emissiveTextureName)
{
    const auto textureId = materialManager_.GetTextureId(emissiveTextureName);
    phongTexturedMaterial_.emissiveMap = textureId;
    return *this;
}


PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetShininessMap(const std::string& shininessTextureName)
{
    const auto textureId = materialManager_.GetTextureId(shininessTextureName);
    phongTexturedMaterial_.shininessMap = textureId;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetOpacityMap(const std::string& opacityTextureName)
{
    const auto textureId = materialManager_.GetTextureId(opacityTextureName);
    phongTexturedMaterial_.opacityMap = textureId;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetAmbientOcclusionMap(const std::string& aoTextureName)
{
    const auto textureId = materialManager_.GetTextureId(aoTextureName);
    phongTexturedMaterial_.aoMap = textureId;
    return *this;
}

PhongTexturedMaterialBuilder& PhongTexturedMaterialBuilder::SetHeightMap(const std::string& heightTextureName)
{
    const auto textureId = materialManager_.GetTextureId(heightTextureName);
    phongTexturedMaterial_.heightMap = textureId;
    return *this;
}

void PhongTexturedMaterialBuilder::Build() { materialManager_.RegisterMaterial(materialName_, phongTexturedMaterial_); }

MaterialManager::MaterialManager(ResourceManager& resourceManager,
                                 const std::shared_ptr<vulkan_wrapper::VulkanCommandPool>& cmdPool,
                                 const std::shared_ptr<vulkan_wrapper::VulkanQueue>& queue)
    : resourceManager_(resourceManager), cmdPool_(cmdPool), queue_(queue)
{
}

void MaterialManager::LoadTexture(const std::string& textureName,
                                  const std::string& samplerName,
                                  const asset_manager::TextureAsset& textureAsset,
                                  const VkFormat& format,
                                  const bool mipmappingEnabled)
{
    // Create texture resource info
    const TextureId textureId = globalTextureId_++;
    const auto textureImageName = textureName + kVulkanImagePostfix;
    const auto textureImageViewName = textureName + kVulkanImageViewPostfix;

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    std::uint32_t mipLevels = 1;
    if (mipmappingEnabled) {
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        mipLevels = utility::GetMipLevelCount(textureAsset.width, textureAsset.height);
    }

    const auto imageResource = ImageResourceCreateInfo{
        .name = textureImageName,
        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .format = format,
        .dimensions = {textureAsset.width, textureAsset.height, 1},
        .mipLevels = mipLevels,
        .usageFlags = usageFlags,
        .views = {ImageViewCreateInfo{.viewName = textureImageViewName,
                                      .format = format,
                                      .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1}}}};

    resourceManager_.CreateImages({imageResource});

    resourceManager_.SetImageFromTexture(cmdPool_, queue_, textureImageName, textureAsset, mipLevels);

    if (mipmappingEnabled) {
        resourceManager_.GenerateMipmaps(cmdPool_, queue_, textureImageName, textureAsset, mipLevels);
    }

    textureHandlers_[textureName] =
            InternalTextureHandler{textureId, textureName, textureImageName, textureImageViewName, samplerName};
}

void MaterialManager::LoadCubemapTexture(const std::string& textureName,
                                         const std::string& samplerName,
                                         const asset_manager::TextureAsset& rightTextureAsset,
                                         const asset_manager::TextureAsset& leftTextureAsset,
                                         const asset_manager::TextureAsset& topTextureAsset,
                                         const asset_manager::TextureAsset& bottomTextureAsset,
                                         const asset_manager::TextureAsset& backTextureAsset,
                                         const asset_manager::TextureAsset& frontTextureAsset,
                                         const VkFormat& format)
{
    // Create texture resource info
    const TextureId textureId = globalTextureId_++;
    const auto textureImageName = textureName + kVulkanImagePostfix;
    const auto textureImageViewName = textureName + kVulkanImageViewPostfix;

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    const auto imageResource = ImageResourceCreateInfo{
        .name = textureImageName,
        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .createFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
        .format = format,
        .dimensions = {rightTextureAsset.width, rightTextureAsset.height, 1},
        .mipLevels = 1,
        .arrayLayers = 6,
        .usageFlags = usageFlags,
        .views = {ImageViewCreateInfo{.viewName = textureImageViewName,
                                      .viewType = VK_IMAGE_VIEW_TYPE_CUBE,
                                      .format = format,
                                      .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6}}}};

    resourceManager_.CreateImages({imageResource});

    resourceManager_.SetImageFromTexture(cmdPool_, queue_, textureImageName, rightTextureAsset, 1, 0);
    resourceManager_.SetImageFromTexture(cmdPool_, queue_, textureImageName, leftTextureAsset, 1, 1);
    resourceManager_.SetImageFromTexture(cmdPool_, queue_, textureImageName, topTextureAsset, 1, 2);
    resourceManager_.SetImageFromTexture(cmdPool_, queue_, textureImageName, bottomTextureAsset, 1, 3);
    resourceManager_.SetImageFromTexture(cmdPool_, queue_, textureImageName, backTextureAsset, 1, 4);
    resourceManager_.SetImageFromTexture(cmdPool_, queue_, textureImageName, frontTextureAsset, 1, 5);

    cubemapTextureHandlers_[textureName] =
            InternalTextureHandler{textureId, textureName, textureImageName, textureImageViewName, samplerName};
}

TextureId MaterialManager::GetTextureId(const std::string& textureName)
{
    return textureHandlers_[textureName].textureId;
}

std::uint32_t MaterialManager::GetTextureCount() const { return textureHandlers_.size(); }

std::uint32_t MaterialManager::GetCubemapTextureCount() const { return cubemapTextureHandlers_.size(); }

std::vector<VkDescriptorImageInfo> MaterialManager::GetDescriptorImageInfos() const
{
    std::vector<VkDescriptorImageInfo> descriptorImageInfos;
    descriptorImageInfos.resize(textureHandlers_.size());

    for (const auto& textureHandler: textureHandlers_) {
        const auto internalHandler = textureHandler.second;
        const auto textureId = internalHandler.textureId;
        const auto sampler = resourceManager_.GetSampler(internalHandler.samplerResourceName);
        const auto imageView =
                resourceManager_.GetImageView(internalHandler.imageResourceName, internalHandler.imageViewResourceName);
        descriptorImageInfos[textureId].sampler = sampler->GetHandle();
        descriptorImageInfos[textureId].imageView = imageView->GetHandle();
        descriptorImageInfos[textureId].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    return descriptorImageInfos;
}

std::vector<VkDescriptorImageInfo> MaterialManager::GetCubemapDescriptorImageInfo(const std::string& textureName)
{
    const auto& internalInfo = cubemapTextureHandlers_[textureName];

    std::vector<VkDescriptorImageInfo> descriptorImageInfos;
    const auto sampler = resourceManager_.GetSampler(internalInfo.samplerResourceName);
    const auto imageView =
            resourceManager_.GetImageView(internalInfo.imageResourceName, internalInfo.imageViewResourceName);
    descriptorImageInfos.emplace_back(sampler->GetHandle(), imageView->GetHandle(),
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return descriptorImageInfos;
}

PhongMaterialBuilder MaterialManager::CreatePhongMaterial(const std::string& materialName)
{
    return {*this, materialName};
}

PhongTexturedMaterialBuilder MaterialManager::CreatePhongTexturedMaterial(const std::string& materialName)
{
    return {*this, materialName};
}

PhongMaterial& MaterialManager::GetPhongMaterial(const std::string& materialName)
{
    return std::get<PhongMaterial>(materialHandlers_[materialName]);
}

PhongTexturedMaterial& MaterialManager::GetPhongTexturedMaterial(const std::string& materialName)
{
    return std::get<PhongTexturedMaterial>(materialHandlers_[materialName]);
}

void MaterialManager::RegisterMaterial(const std::string& materialName, const MaterialVariant& material)
{
    materialHandlers_[materialName] = material;
}

} // namespace common::vulkan_framework
