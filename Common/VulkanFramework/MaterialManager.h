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
#include "TextureAsset.h"

namespace common::vulkan_framework
{

class MaterialManager;

using TextureId = std::int32_t;
using SamplerId = uint32_t;

struct InternalTextureHandler
{
    TextureId textureId = -1;
    std::string textureName;
    std::string imageResourceName;
    std::string imageViewResourceName;
    std::string samplerResourceName;
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

private:
    static constexpr auto kVulkanImagePostfix = "_image";
    static constexpr auto kVulkanImageViewPostfix = "_imageView";

    ResourceManager& resourceManager_;
    std::shared_ptr<vulkan_wrapper::VulkanCommandPool> cmdPool_;
    std::shared_ptr<vulkan_wrapper::VulkanQueue> queue_;

    std::int32_t globalTextureId_ = 0;
    std::unordered_map<std::string, InternalTextureHandler> textureHandlers_;
    std::unordered_map<std::string, InternalTextureHandler> cubemapTextureHandlers_;
};

} // namespace common::vulkan_framework
