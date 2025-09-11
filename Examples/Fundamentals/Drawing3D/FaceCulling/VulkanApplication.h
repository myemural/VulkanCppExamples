/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and ApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    11.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationData.h"
#include "ApplicationDrawing3D.h"
#include "ShaderLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"
#include "TextureLoader.h"

namespace examples::fundamentals::drawing_3d::face_culling
{

// User customizable settings
struct ApplicationSettings
{
    VkClearColorValue ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    float MouseSensitivity = 1.0f;
    float CameraSpeed = 1.0f;
    VkCullModeFlags CullMode = VK_CULL_MODE_NONE;
    VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
};

// Project constants
inline constexpr std::uint32_t kMaxFramesInFlight = 2;
inline constexpr auto kCurrentShaderType = common::utility::ShaderBaseType::GLSL;
inline constexpr auto kVertexShaderFileName = "drawing_cube.vert.spv";
inline constexpr auto kFragmentShaderFileName = "drawing_cube.frag.spv";
inline constexpr auto kVertexShaderHash = "vertMain";
inline constexpr auto kFragmentShaderHash = "fragMain";

// Buffer keys
inline constexpr auto kVertexBufferKey = "mainVertexBuffer";
inline constexpr auto kIndexBufferKey = "mainIndexBuffer";
inline constexpr auto kTextureStagingBufferKey = "textureStagingBuffer";

// Image, image view and sampler keys
inline constexpr auto kCrateImageKey = "createImage";
inline constexpr auto kCrateImageViewKey = "crateImageView";
inline constexpr auto kDepthImageKey = "depthImage";
inline constexpr auto kDepthImageViewKey = "depthImageView";
inline constexpr auto kMainSamplerKey = "mainSampler";

// Descriptor layout keys
inline constexpr auto kMainDescSetLayoutKey = "mainDescSetLayout";

// Texture paths
inline constexpr auto kTextureCratePath = "Textures/crate1_diffuse.png";

class VulkanApplication final : public base::ApplicationDrawing3D
{
public:
    VulkanApplication(const common::vulkan_framework::ApplicationCreateConfig &config, const ApplicationSettings& settings);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void Update() override;

    void Cleanup() override;

    void InitInputSystem();

    void InitResources(const VkFormat& depthImageFormat);

    void CreateRenderPass(const VkFormat& depthImageFormat);

    void CreatePipeline();

    void UpdateDescriptorSets();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex, std::uint32_t indexCount);

    void CalculateAndSetMvp();

    void ProcessInput();

    ApplicationSettings settings_;
    std::uint32_t currentIndex_ = 0;
    bool keys_[1024]= {};
    MvpData mvpData[NUM_CUBES] = {glm::mat4(1.0)};

    // Create infos
    std::vector<base::BufferCreateInfo> bufferCreateInfos_;
    std::vector<common::vulkan_framework::ImageResourceCreateInfo> imageResourceCreateInfos_;
    std::vector<common::vulkan_framework::SamplerResourceCreateInfo> samplerResourceCreateInfos_;
    base::ShaderModulesCreateInfo shaderModuleCreateInfo_{};
    base::DescriptorSetCreateInfo descriptorSetCreateInfo_{};
    base::DescriptorSetUpdateInfo descriptorSetUpdateInfo_{};

    // Texture resource
    common::utility::TextureHandler crateTextureHandler_{};

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;
    std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> cmdBufferTransfer_;

    // Camera values
    glm::vec3 cameraPos_ = glm::vec3(0.0f, 0.0f, 4.0f);   // Camera position
    glm::vec3 cameraFront_ = glm::vec3(0.0f, 0.0f, -1.0f); // Front position
    glm::vec3 cameraUp_ = glm::vec3(0.0f, 1.0f, 0.0f);    // Up vector

    // Delta time related values
    float deltaTime_ = 0.0f;
    float lastFrame_ = 0.0f;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float yawAngle_ = -90.0f;
    float pitchAngle_ = 0.0f;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;
};
} // namespace examples::fundamentals::drawing_3d::face_culling