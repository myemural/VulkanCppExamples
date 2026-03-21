# Tiled Forward Shading (Forward+)

**Code Name:** TiledForwardShading

## Description

This example uses a tiled forward (forward+) shading approach to apply lighting to objects drawn on the screen.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/LightingArchitectures/TiledForwardShading.png?raw=true)

## Controls

| Input   | Action                               |
|---------|--------------------------------------|
| W/A/S/D | Move the camera                      |
| Mouse   | Look around with the camera          |
| Esc     | Close the window                     |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |
| AppSettings.LightRadius      | float             | AppSettings::LightRadius      | Radius value of the light source |               |


## Learning Objectives

- Implementing tiled forward shading (forward+) technique in Vulkan
- Compute light list per tile with compute shaders (limited light count per tile)

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :x:                | Will be implemented. |
| Slang       | :x:                | Will be implemented. |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
