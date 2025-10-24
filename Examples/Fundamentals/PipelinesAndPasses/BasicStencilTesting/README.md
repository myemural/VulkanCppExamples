# Object Outlining with Stencil Testing

**Code Name:** BasicStencilTesting

## Description

In this example, outlines are drawn around objects using multiple pipelines and stencil testing.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/PipelinesAndPasses/BasicStencilTesting.png?raw=true)

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
| AppSettings.OutlineWidth     | float             | AppSettings::OutlineWidth     | Thickness value of the outline |               |


## Learning Objectives

- Enabling stencil testing
- Understanding stencil testing operations
- Implementing object outline using stencil testing

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
