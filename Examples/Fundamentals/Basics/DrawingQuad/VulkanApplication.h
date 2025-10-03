/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and VulkanApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    21.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationBasics.h"
#include "ParameterServer.h"
#include "ShaderLoader.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDeviceMemory.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "VulkanShaderModule.h"
#include "Window.h"

namespace examples::fundamentals::basics::drawing_quad
{
class VulkanApplication final : public base::ApplicationBasics
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

protected:
    bool Init() override;

    void DrawFrame() override;

private:
    void CreateResources();

    void InitResources() const;

    void CreateVertexBuffer(std::uint64_t dataSize);

    void FillVertexBuffer(const void* data, std::uint64_t dataSize) const;

    void CreateIndexBuffer(std::uint64_t dataSize);

    void FillIndexBuffer(const void* data, std::uint64_t dataSize) const;

    void CreateShaderModules();

    void CreatePipeline();

    void CreateCommandBuffers();

    void RecordCommandBuffers(std::uint32_t indexCount);

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;

    std::shared_ptr<common::vulkan_wrapper::VulkanBuffer> vertexBuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDeviceMemory> vertexBufferMemory_;
    std::shared_ptr<common::vulkan_wrapper::VulkanBuffer> indexBuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDeviceMemory> indexBufferMemory_;
    std::unordered_map<std::string, std::shared_ptr<common::vulkan_wrapper::VulkanShaderModule>> shaderModules_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffers_;
};
} // namespace examples::fundamentals::basics::drawing_quad
