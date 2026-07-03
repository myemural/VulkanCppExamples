# Anisotropic Filtering with Variance Shadow Maps

**Code Name:** AnisotropicShadowMaps

## Description

In this example, the mipmaps of the generated variance shadow map are created, and the gradient-based shadow map LOD selection is made according to the camera's distance from the fragment with anisotropic filtering.

## Output

![](/Docs/ExampleMedia/RealTimeShadows/ShadowResourceManagement/AnisotropicShadowMaps.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |
| 0       | Disable shadow map LOD      |
| 1       | Enable shadow map LOD       |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Apply anisotropic filtering to the mipmapped shadow map to achieve better soft shadows by distances to the camera

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
