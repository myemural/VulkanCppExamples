# Screen-Space Ambient Occlusion (SSAO)

**Code Name:** ScreenSpaceAo

## Description

In this example, Screen-Space Ambient Occlusion (SSAO) technique has been implemented and demonstrated with an example scene. The implementation has been done using deferred shading and 2 additional passes (SSAO and SSAO Blur), providing numerous debugging opportunities.

## Output

![](/Docs/ExampleMedia/PostProcessingEffects/AmbientOcclusion/ScreenSpaceAo.png?raw=true)

## Controls

| Input   | Action                             |
|---------|------------------------------------|
| W/A/S/D | Move the camera                    |
| Mouse   | Look around with the camera        |
| Esc     | Close the window                   |
| 0       | Debug output off                   |
| 1       | Debug output for albedo            |
| 2       | Debug output for view-space normal |
| 3       | Debug output for SSAO raw          |
| 4       | Debug output for SSAO blur         |
| 5       | Debug output for ambient only      |
| Space   | Enable/disable SSAO                |
| Z       | Decrease SSAO radius               |
| X       | Increase SSAO radius               |
| C       | Decrease SSAO bias                 |
| V       | Increase SSAO bias                 |
| B       | Decrease SSAO power                |
| N       | Increase SSAO power                |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Understanding how to implement Screen-Space Ambient Occlusion (SSAO)
- Implementing SSAO and SSAO Blur passes in Vulkan

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
