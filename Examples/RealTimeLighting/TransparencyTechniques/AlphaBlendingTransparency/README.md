# Order-Dependent Transparency with Alpha Blending

**Code Name:** AlphaBlendingTransparency

## Description

In this example, four objects are drawn consecutively on the scene, three of which are assigned a transparent material, and the objects are drawn in order according to the camera distance. Alpha blending is applied to these objects to simulate transparency.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/TransparencyTechniques/AlphaBlendingTransparency.png?raw=true)

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

## Learning Objectives

- Implementing order-dependent transparency with ordinary alpha blending in Vulkan
- Ordering objects in the scene to the camera distance
- Understanding the blending coefficients and operations in the Vulkan pipelines

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
