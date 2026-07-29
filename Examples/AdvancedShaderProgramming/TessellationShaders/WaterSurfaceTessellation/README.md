# Simple Water Surface Simulation via Tessellation Shader

**Code Name:** WaterSurfaceTessellation

## Description

In this example, a simple water wave simulation is performed using Gerstner waves (Trochoidal waves) on a plane tessellated with tessellation shaders.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/TessellationShaders/WaterSurfaceTessellation.png?raw=true)

## Controls

| Input   | Action                           |
|---------|----------------------------------|
| W/A/S/D | Move the camera                  |
| Mouse   | Look around with the camera      |
| Esc     | Close the window                 |

## Application Parameters

### Settings

| Parameter / Key               | Type              | Usage in Code                  | Description                    | Default Value |
|-------------------------------|-------------------|--------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor        | VkClearColorValue | AppSettings::ClearColor        | Background color of the screen |               |
| AppSettings.MouseSensitivity  | float             | AppSettings::MouseSensitivity  | Mouse sensitivity value        |               |
| AppSettings.CameraSpeed       | float             | AppSettings::CameraSpeed       | Speed of the camera            |               |
| AppSettings.PolygonMode       | VkPolygonMode     | AppSettings::PolygonMode       | Polygon mode                   |               |
| AppSettings.TessLevel         | float             | AppSettings::TessLevel         | Tessellation level             |               |
| AppSettings.DisplacementLevel | float             | AppSettings::DisplacementLevel | Displacement level             |               |

## Learning Objectives

- Implementing basic water (or ocean) surface simulation with using Gerstner waves on tessellated plane object
- Implementing the water foam effect that appears when the wave height increases

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
