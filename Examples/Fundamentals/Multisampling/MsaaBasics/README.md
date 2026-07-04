# MSAA Basics

**Code Name:** MsaaBasics

## Description

In this example, MSAA is activated based on the maximum sample number supported by the system, and the color and depth images are handled as multisampled images. Render pass was used for automatic resolving.

## Output

![](/Docs/ExampleMedia/Fundamentals/Multisampling/MsaaBasics.png?raw=true)

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

- Enabling multisampling in Vulkan
- Getting maximum supported sample count of your window system
- Implementing basic MSAA

## Shader Status

| Shader Type                                                   | Status             | Notes |
|---------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/Multisampling/MsaaBasics/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/Multisampling/MsaaBasics/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/Multisampling/MsaaBasics/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
