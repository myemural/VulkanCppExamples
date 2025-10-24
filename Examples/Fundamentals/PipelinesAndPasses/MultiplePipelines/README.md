# Using Multiple Pipelines and Pipeline Derivatives

**Code Name:** MultiplePipelines

## Description

In this example, multiple cubes are drawn in the scene. The first 3 cubes in the scene are drawn with a pipeline that gives red output, the last 3 cubes are drawn with a pipeline that gives blue output, and the remaining cubes are drawn with a pipeline that gives wireframe output. Additionally, the pipeline that gives blue output was created with pipeline derivative.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/PipelinesAndPasses/MultiplePipelines.png?raw=true)

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

- Creating and using multiple pipelines in one render pass and one subpass
- Understanding pipeline derivatives
- Creating derived pipeline

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
