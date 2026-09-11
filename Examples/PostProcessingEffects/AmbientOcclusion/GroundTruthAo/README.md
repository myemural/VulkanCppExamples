# Ground-Truth Ambient Occlusion (GTAO)

**Code Name:** GroundTruthAo

## Description

This example implements Ground-Truth Ambient Occlusion (GTAO) on top of a deferred shading architecture. The occlusion factor is calculated in a separate pass by marching the horizon on both sides of a set of view-space slices, then solving the cosine-weighted visibility integral of every slice analytically.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/AmbientOcclusion/GroundTruthAo.png?raw=true)

## Controls

| Input   | Action                             |
|---------|------------------------------------|
| W/A/S/D | Move the camera                    |
| Mouse   | Look around with the camera        |
| Esc     | Close the window                   |
| 0       | Debug output off                   |
| 1       | Debug output for albedo            |
| 2       | Debug output for view-space normal |
| 3       | Debug output for GTAO raw          |
| 4       | Debug output for GTAO blur         |
| 5       | Debug output for ambient only      |
| Space   | Enable/disable GTAO                |
| Z       | Decrease GTAO radius               |
| X       | Increase GTAO radius               |
| C       | Decrease GTAO thickness            |
| V       | Increase GTAO thickness            |
| B       | Decrease GTAO power                |
| N       | Increase GTAO power                |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement Ground-Truth Ambient Occlusion (GTAO)
- Implementing GTAO and GTAO Blur passes in Vulkan

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
