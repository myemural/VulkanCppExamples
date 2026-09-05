# Real-Time Ray Tracing using BVH Traversing with Compute Shader

**Code Name:** RayTracingCompute

## Description

In this example, the whole scene is rendered by a compute shader that traverses a binned SAH Bounding Volume Hierarchy (BVH) built on the host from the scene objects. Primary rays, area light shadow rays and a single specular bounce are all traced in the same dispatch.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/AdvancedComputeShaders/RayTracingCompute.png?raw=true)

## Controls

| Input   | Action                               |
|---------|--------------------------------------|
| W/A/S/D | Move the camera                      |
| Mouse   | Look around with the camera          |
| Esc     | Close the window                     |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Creating Bounding Volume Hierarchy (BVH) in Vulkan
- Implementing GPU-based real-time ray-tracing using compute shaders

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
