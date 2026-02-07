/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    18.01.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <array>
#include <memory>

#include "ApplicationData.h"
#include "ApplicationQueriesAndPerformance.h"
#include "AssetManager.h"
#include "PerspectiveCamera.h"
#include "TextureAsset.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"

namespace examples::fundamentals::queries_and_performance::descriptor_indexing
{
class VulkanApplication final : public base::ApplicationQueriesAndPerformance
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void PreUpdate() override;

private:
    void InitInputSystem();

    void PrepareBufferInfos();

    void InitAssetManager();

    void CreateResources();

    void InitResources() const;

    void CreateRenderPass();

    void CreatePipelines();

    void UpdateDescriptorSets() const;

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void CalculateAndSetMvp();

    void ProcessInput() const;

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    VkFormat depthImageFormat_ = VK_FORMAT_UNDEFINED;
    std::array<ObjectUbo, MAX_NUM_OBJECTS> objectUbo_ = {glm::mat4(1.0)};
    std::uint32_t uboAlignedSize_ = UINT32_MAX;

    // Buffer (vertex/index) related data
    std::array<PrimitiveData, PRIMITIVE_TYPE_COUNT> primitivesData_ = {};
    std::array<BufferAllocInfo, PRIMITIVE_TYPE_COUNT> bufferAllocInfos_ = {};
    std::uint32_t totalBufferSize_ = UINT32_MAX;

    // Texture resource
    common::asset_manager::TextureAsset crateTextureAsset_{};
    common::asset_manager::TextureAsset wallStoneTextureAsset_{};

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Asset manager
    std::unique_ptr<common::asset_manager::AssetManager> assetManager_;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;

    // Camera
    std::unique_ptr<common::utility::PerspectiveCamera> camera_;
};
} // namespace examples::fundamentals::queries_and_performance::descriptor_indexing
