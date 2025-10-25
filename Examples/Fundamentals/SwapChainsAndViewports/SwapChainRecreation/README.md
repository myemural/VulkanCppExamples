# Handling Window Resizing

**Code Name:** SwapChainRecreation

## Description

In this example, cubes are drawn on the screen and navigated using the camera. If the window size is changed, the swap chain object is recreated and the drawing is reapplied according to the new window size.

## Screenshots / Recordings

![](/Docs/ExampleMedia/Fundamentals/SwapChainsAndViewports/SwapChainRecreation.png?raw=true)

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

- Handling window resizing in Vulkan
- Recreating the swap chain

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
