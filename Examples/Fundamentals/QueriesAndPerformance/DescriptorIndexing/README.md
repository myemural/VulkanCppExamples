# Descriptor Indexing

**Code Name:** DescriptorIndexing

## Description

This example demonstrates descriptor indexing, which allows multiple textures to be handled bindless by the shader.

## Output

![](/Docs/ExampleMedia/Fundamentals/QueriesAndPerformance/DescriptorIndexing.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                    | Default Value |
|------------------------------|-------------------|-------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value        |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera            |               |


## Learning Objectives

- Implementing and using descriptor indexing feature in Vulkan
- Using variable sized sampler arrays in shaders
- Access variable sized sampler arrays via non-uniform indexing

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :white_check_mark: |                      |
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
