# Hierarchical Z-Buffer Occlusion Culling with Compute Shader

**Code Name:** OcclusionCullingCompute

## Description

This example aims to reduce the number of draw calls by using frustum culling and occlusion culling with a compute shader. The user can enable and disable frustum culling and occlusion culling and observe the statistics (FPS, draw call count, occluded objects etc.).

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/AdvancedComputeShaders/OcclusionCullingCompute.png?raw=true)

## Controls

| Input   | Action                           |
|---------|----------------------------------|
| W/A/S/D | Move the camera                  |
| Mouse   | Look around with the camera      |
| Esc     | Close the window                 |
| F       | Enable/disable frustum culling   |
| O       | Enable/disable occlusion culling |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement frustum culling and occlusion culling using compute shaders
- Implementing Hierarchical Z-Buffer Occlusion Culling technique in Vulkan

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :x:                | Will be implemented. |
| Slang       | :x:                | Will be implemented. |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
