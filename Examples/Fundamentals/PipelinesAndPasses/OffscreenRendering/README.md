# Rendering Scene to a Texture

**Code Name:** OffscreenRendering

## Description

In this example, a quad is placed on the right side of the screen and the rendered scene is drawn as a texture on this quad with offscreen rendering.

## Output

![](/Docs/ExampleMedia/Fundamentals/PipelinesAndPasses/OffscreenRendering.png?raw=true)

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

- Implementing offscreen rendering using multiple pipelines and render passes
- Rendering a mini camera view on the screen

## Shader Status

| Shader Type                                                                | Status             | Notes |
|----------------------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/PipelinesAndPasses/OffscreenRendering/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/PipelinesAndPasses/OffscreenRendering/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/PipelinesAndPasses/OffscreenRendering/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
