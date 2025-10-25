# Split Screen with Dynamic Viewport

**Code Name:** DynamicViewport

## Description

In this example, two viewports are placed on the left and right of the screen. The left viewport displays a perspective view of the scene, and the right viewport displays an orthographic view. The viewports are dynamically generated.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/SwapChainsAndViewports/DynamicViewport.png?raw=true)

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

- Setting viewports dynamically
- Rendering scene with orthographic projection

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
