# Pipeline Caches

**Code Name:** PipelineCaches

## Description

In this example, the pipeline's cache information is saved to disk when the application exits. Later, if this file is found on disk, the pipeline is created by loading it from this cache.

## Output

![](/Docs/ExampleMedia/Fundamentals/QueriesAndPerformance/PipelineCaches.png?raw=true)

**Console Output**
```bash
Pipeline cache loaded!
examples::fundamentals::queries_and_performance::pipeline_caches::VulkanApplication::CreatePipeline: 463 microseconds
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

- Using pipeline caches in Vulkan
- Saving pipeline to a cache file
- Loading pipelines from a cache if the cache is available

## Shader Status

| Shader Type                                                               | Status             | Notes |
|---------------------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/QueriesAndPerformance/PipelineCaches/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/QueriesAndPerformance/PipelineCaches/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/QueriesAndPerformance/PipelineCaches/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
