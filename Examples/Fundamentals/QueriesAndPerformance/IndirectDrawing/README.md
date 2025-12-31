# Reduce CPU Bottleneck with Indirect Drawing

**Code Name:** IndirectDrawing

## Description

In this example, many primitives of 4 different types are drawn on the screen with a single indirect drawing command to reduce CPU draw call bottleneck.

## Output

![](/Docs/ExampleMedia/Fundamentals/QueriesAndPerformance/IndirectDrawing.png?raw=true)

**Console Output**
```bash
examples::fundamentals::queries_and_performance::indirect_drawing::VulkanApplication::DrawFrame: 7041 microseconds
examples::fundamentals::queries_and_performance::indirect_drawing::VulkanApplication::DrawFrame: 7338 microseconds
examples::fundamentals::queries_and_performance::indirect_drawing::VulkanApplication::DrawFrame: 6689 microseconds
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

- Using indirect drawing technique in Vulkan
- Learning basic GPU-driven drawing
- Using indexed indirect drawing

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
