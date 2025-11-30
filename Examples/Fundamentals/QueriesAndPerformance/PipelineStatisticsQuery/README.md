# Measuring Pipeline Statistics

**Code Name:** PipelineStatisticsQuery

## Description

In this example, 20 cubes in four different groups are drawn on the screen using four different pipelines. Then, using the pipeline statistics query, information about these 4 pipelines is printed to the console.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/QueriesAndPerformance/PipelineStatisticsQuery.png?raw=true)

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

- Using pipeline statistics query in Vulkan
- Measuring input assembly vertices and primitives count of a pipeline
- Measuring shader invocation counts of a pipeline

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
