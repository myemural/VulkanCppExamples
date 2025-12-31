# Buffer Suballocation

**Code Name:** BufferSuballocation

## Description

In this example, many objects are drawn to the screen using buffer suballocation, each with four different vertex and four different index data. A single buffer is used to hold the vertex and index data.

## Output

![](/Docs/ExampleMedia/Fundamentals/QueriesAndPerformance/BufferSuballocation.png?raw=true)

**Console Output**
```bash
examples::fundamentals::queries_and_performance::buffer_suballocation::VulkanApplication::DrawFrame: 12281 microseconds
examples::fundamentals::queries_and_performance::buffer_suballocation::VulkanApplication::DrawFrame: 12065 microseconds
examples::fundamentals::queries_and_performance::buffer_suballocation::VulkanApplication::DrawFrame: 11759 microseconds
...
```

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

- Using buffer suballocation technique in Vulkan
- Using one big buffer for different vertex and index data

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
