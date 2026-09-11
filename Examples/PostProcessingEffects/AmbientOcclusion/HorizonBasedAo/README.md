# Horizon-Based Ambient Occlusion (HBAO)

**Code Name:** HorizonBasedAo

## Description

This example implements Horizon-Based Ambient Occlusion (HBAO) on top of a deferred shading architecture. The occlusion factor is calculated in a separate pass by marching along randomly rotated screen-space directions over the view-space position G-Buffer and tracking the highest horizon angle per direction, then it is blurred.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/AmbientOcclusion/HorizonBasedAo.png?raw=true)

## Controls

| Input   | Action                             |
|---------|------------------------------------|
| W/A/S/D | Move the camera                    |
| Mouse   | Look around with the camera        |
| Esc     | Close the window                   |
| 0       | Debug output off                   |
| 1       | Debug output for albedo            |
| 2       | Debug output for view-space normal |
| 3       | Debug output for HBAO raw          |
| 4       | Debug output for HBAO blur         |
| 5       | Debug output for ambient only      |
| Space   | Enable/disable HBAO                |
| Z       | Decrease HBAO radius               |
| X       | Increase HBAO radius               |
| C       | Decrease HBAO angle bias           |
| V       | Increase HBAO angle bias           |
| B       | Decrease HBAO power                |
| N       | Increase HBAO power                |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement Horizon-Based Ambient Occlusion (HBAO)
- Implementing HBAO and HBAO Blur passes in Vulkan

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
