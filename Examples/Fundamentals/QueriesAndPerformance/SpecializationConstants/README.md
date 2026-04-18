# Specialization Constants

**Code Name:** SpecializationConstants

## Description

In this example, three different variants of a fragment shader are created using specialization constants, and three separate pipelines are created from these fragment shaders. Then drawing is done on the screen with these pipelines.

## Output

![](/Docs/ExampleMedia/Fundamentals/QueriesAndPerformance/SpecializationConstants.png?raw=true)

**Console Output**
```bash
examples::fundamentals::queries_and_performance::specialization_constants::VulkanApplication::DrawFrame: 11051 microseconds
examples::fundamentals::queries_and_performance::specialization_constants::VulkanApplication::DrawFrame: 11898 microseconds
examples::fundamentals::queries_and_performance::specialization_constants::VulkanApplication::DrawFrame: 11392 microseconds
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

- Using specialization constants in Vulkan and shaders
- Creating pipelines with giving values to specialization constants

## Shader Status

| Shader Type | Status             | Notes |
|-------------|--------------------|-------|
| GLSL        | :white_check_mark: |       |
| HLSL        | :white_check_mark: |       |
| Slang       | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
