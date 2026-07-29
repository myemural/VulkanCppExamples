# Displacement Mapping with Tessellation Shaders

**Code Name:** DisplacementMappingTessellation

## Description

In this example, displacement mapping technique is applied to an object tessellated with tessellation shaders. Additionally, an extra calculation is performed in the evaluation shader for spherical objects.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/TessellationShaders/DisplacementMappingTessellation.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |
| AppSettings.PolygonMode      | VkPolygonMode     | AppSettings::PolygonMode      | Polygon mode                     |               |

## Learning Objectives

TBD

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :x:                | Will be implemented. |
| Slang       | :x:                | Will be implemented. |

## Theoretical Background

- Learning how to implement displacement mapping with tessellation shaders
- Understanding the interaction between tessellation and displacement techniques

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
