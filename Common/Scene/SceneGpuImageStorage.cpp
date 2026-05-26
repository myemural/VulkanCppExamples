/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "SceneGpuImageStorage.h"

#include "MathUtils.h"

namespace common::scene
{

using namespace common::asset_manager;
using namespace common::vulkan_framework;
using namespace common::vulkan_wrapper;


SceneGpuImageStorage::SceneGpuImageStorage(ResourceManager& resourceManager, const SceneConfig& sceneConfig)
    : resourceManager_(resourceManager),
      sceneConfig_(sceneConfig),
      cmdPool_(sceneConfig.imageTransferCmdPool),
      queue_(sceneConfig.imageTransferQueue)
{
}

TextureId SceneGpuImageStorage::StoreTexture(const std::string& textureName,
                                             const std::string& samplerName,
                                             const TextureAsset& textureAsset,
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

    return textureId;
}

TextureId SceneGpuImageStorage::StoreTexture(const std::string& textureName,
                                             const std::string& samplerName,
                                             const TextureAssetHDR& textureAsset,
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

    /// TODO: Fix mipmap generation on resource manager and uncomment these lines.
    // if (mipmappingEnabled) {
    //     resourceManager_.GenerateMipmaps(cmdPool_, queue_, textureImageName, textureAsset, mipLevels);
    // }

    textureHandlers_[textureName] =
            InternalTextureHandler{textureId, textureName, textureImageName, textureImageViewName, samplerName};

    return textureId;
}

TextureId SceneGpuImageStorage::StoreCubemapTexture(const std::string& textureName,
                                                    const std::string& samplerName,
                                                    const TextureAsset& rightTextureAsset,
                                                    const TextureAsset& leftTextureAsset,
                                                    const TextureAsset& topTextureAsset,
                                                    const TextureAsset& bottomTextureAsset,
                                                    const TextureAsset& backTextureAsset,
                                                    const TextureAsset& frontTextureAsset,
                                                    const VkFormat& format)
{
    // Create texture resource info
    const TextureId textureId = globalTextureId_++;
    const auto textureImageName = textureName + kVulkanImagePostfix;
    const auto textureImageViewName = textureName + kVulkanImageViewPostfix;

    const auto imageResource = ImageResourceCreateInfo{
        .name = textureImageName,
        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .createFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
        .format = format,
        .dimensions = {rightTextureAsset.width, rightTextureAsset.height, 1},
        .mipLevels = 1,
        .arrayLayers = 6,
        .usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
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

    return textureId;
}
TextureId SceneGpuImageStorage::GetTextureId(const std::string& textureName)
{
    return textureHandlers_[textureName].textureId;
}

std::uint32_t SceneGpuImageStorage::GetTextureCount() const { return textureHandlers_.size(); }

std::uint32_t SceneGpuImageStorage::GetCubemapTextureCount() const { return cubemapTextureHandlers_.size(); }

std::vector<VkDescriptorImageInfo> SceneGpuImageStorage::GetDescriptorImageInfo(const std::string& textureName)
{
    const auto& internalInfo = textureHandlers_[textureName];

    std::vector<VkDescriptorImageInfo> descriptorImageInfos;
    const auto sampler = resourceManager_.GetSampler(internalInfo.samplerResourceName);
    const auto imageView =
            resourceManager_.GetImageView(internalInfo.imageResourceName, internalInfo.imageViewResourceName);
    descriptorImageInfos.emplace_back(sampler->GetHandle(), imageView->GetHandle(),
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return descriptorImageInfos;
}

std::vector<VkDescriptorImageInfo> SceneGpuImageStorage::GetDescriptorImageInfos() const
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

std::vector<VkDescriptorImageInfo> SceneGpuImageStorage::GetCubemapDescriptorImageInfo(const std::string& textureName)
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

} // namespace common::scene
