# Clustered Deferred Shading

**Code Name:** ClusteredDeferredShading

## Description

This example demonstrates the clustered deferred shading approach without limiting the amount of light per cluster to apply lighting to objects drawn on the screen.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/LightingArchitectures/ClusteredDeferredShading.png?raw=true)

## Controls

| Input   | Action                               |
|---------|--------------------------------------|
| W/A/S/D | Move the camera                      |
| Mouse   | Look around with the camera          |
| Esc     | Close the window                     |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                       | Default Value |
|------------------------------|-------------------|-------------------------------|-----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen    |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value           |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera               |               |
| AppSettings.CameraNearPlane  | float             | AppSettings::CameraNearPlane  | Near plane distance to the camera |               |
| AppSettings.CameraFarPlane   | float             | AppSettings::CameraFarPlane   | Far plane distance to the camera  |               |
| AppSettings.LightRadius      | float             | AppSettings::LightRadius      | Radius value of the light source  |               |


## Learning Objectives

- Implementing clustered deferred shading technique in Vulkan without limiting the amount of light per cluster

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :white_check_mark: |                      |
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
