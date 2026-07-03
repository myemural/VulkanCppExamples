# Mipmapped Variance Shadow Maps

**Code Name:** MipmappedShadowMaps

## Description

In this example, the mipmaps of the generated variance shadow map are created, and the shadow map LOD selection is made according to the camera's distance from the fragment. Thus, shadow mapping is done with a small shadow map size for long distances, and simple soft shadows are provided through trilinear filtering.

## Output

![](/Docs/ExampleMedia/RealTimeShadows/ShadowResourceManagement/MipmappedShadowMaps.png?raw=true)

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

- Creating mipmapped variance shadow maps
- Apply trilinear filtering to the mipmapped shadow map to achieve soft shadows by distances to the camera

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
