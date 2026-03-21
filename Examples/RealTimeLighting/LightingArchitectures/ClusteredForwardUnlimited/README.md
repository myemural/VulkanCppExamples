# Clustered Forward Shading with Unlimited Lights per Cluster

**Code Name:** ClusteredForwardUnlimited

## Description

In this example, clustered forward shading is implemented without limiting the amount of light per cluster, and the lighting of objects in the scene is achieved using this technique. It also allows for the use of multiple threads in compute shaders for cluster calculations.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/LightingArchitectures/ClusteredForwardUnlimited.png?raw=true)

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

- Implementing clustered forward shading technique in Vulkan with unlimited light count per cluster

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
