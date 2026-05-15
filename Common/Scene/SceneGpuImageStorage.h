/**
 * @file    SceneGpuImageStorage.h
 * @brief   GPU image storage management for scene data.
 * @author  Mustafa Yemural (myemural)
 * @date    08.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "CoreDefines.h"
#include "ResourceManager.h"
#include "SceneConfig.h"

namespace common::scene
{

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

class COMMON_API SceneGpuImageStorage
{
public:
    SceneGpuImageStorage(vulkan_framework::ResourceManager& resourceManager, const SceneConfig& sceneConfig);

    ~SceneGpuImageStorage() = default;

    /**
     * @brief Stores texture asset on the GPU.
     * @param textureName Name of the texture resource.
     * @param samplerName Name of the sampler.
     * @param textureAsset Asset of the texture resource.
     * @param format Storing format of the texture.
     * @param mipmappingEnabled Specifies whether mipmapping is enabled or not.
     * @return Returns texture ID.
     */
    TextureId StoreTexture(const std::string& textureName,
                           const std::string& samplerName,
                           const asset_manager::TextureAsset& textureAsset,
                           const VkFormat& format = VK_FORMAT_R8G8B8A8_SRGB,
                           bool mipmappingEnabled = false);

    /**
     * @brief Stores cubemap texture asset on the GPU.
     * @param textureName Name of the texture resource.
     * @param samplerName Name of the sampler.
     * @param rightTextureAsset Asset of the right side texture of the cubemap texture resource.
     * @param leftTextureAsset Asset of the left side texture of the cubemap texture resource.
     * @param topTextureAsset Asset of the top side texture of the cubemap texture resource.
     * @param bottomTextureAsset Asset of the bottom side texture of the cubemap texture resource.
     * @param backTextureAsset Asset of the back side texture of the cubemap texture resource.
     * @param frontTextureAsset Asset of the front side texture of the cubemap texture resource.
     * @param format Storing format of the texture.
     * @return Returns texture ID.
     */
    TextureId StoreCubemapTexture(const std::string& textureName,
                                  const std::string& samplerName,
                                  const asset_manager::TextureAsset& rightTextureAsset,
                                  const asset_manager::TextureAsset& leftTextureAsset,
                                  const asset_manager::TextureAsset& topTextureAsset,
                                  const asset_manager::TextureAsset& bottomTextureAsset,
                                  const asset_manager::TextureAsset& backTextureAsset,
                                  const asset_manager::TextureAsset& frontTextureAsset,
                                  const VkFormat& format = VK_FORMAT_R8G8B8A8_SRGB);

    [[nodiscard]] TextureId GetTextureId(const std::string& textureName);

    [[nodiscard]] std::uint32_t GetTextureCount() const;

    [[nodiscard]] std::uint32_t GetCubemapTextureCount() const;

    [[nodiscard]] std::vector<VkDescriptorImageInfo> GetDescriptorImageInfos() const;

    [[nodiscard]] std::vector<VkDescriptorImageInfo> GetCubemapDescriptorImageInfo(const std::string& textureName);

private:
    static constexpr auto kVulkanImagePostfix = "_image";
    static constexpr auto kVulkanImageViewPostfix = "_imageView";

    vulkan_framework::ResourceManager& resourceManager_;
    SceneConfig sceneConfig_;
    std::shared_ptr<vulkan_wrapper::VulkanCommandPool> cmdPool_;
    std::shared_ptr<vulkan_wrapper::VulkanQueue> queue_;

    std::int32_t globalTextureId_ = 0;
    std::unordered_map<std::string, InternalTextureHandler> textureHandlers_;
    std::unordered_map<std::string, InternalTextureHandler> cubemapTextureHandlers_;
};

} // namespace common::scene
