# Using Multi-Viewports for Different Colors

**Code Name:** MultiViewport

## Description

In this example, four fixed viewports are placed on the screen, and each viewport's clear color is set differently. The same scene is then drawn in these viewports, and a viewport-specific color mix is applied to the objects.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/SwapChainsAndViewports/MultiViewport.png?raw=true)

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
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value        |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera            |               |


## Learning Objectives

- Rendering to multiple viewport through single pipeline
- Understanding what should we do on shaders to render multiple viewports

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
- VK_EXT_shader_viewport_index_layer
