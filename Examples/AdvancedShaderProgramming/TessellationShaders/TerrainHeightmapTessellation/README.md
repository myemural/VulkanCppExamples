# Terrain Creation via Heightmap using Tessellation Shaders

**Code Name:** TerrainHeightmapTessellation

## Description

This example demonstrates how to create terrain using heightmaps with tessellation shaders and how to use quad tessellation in tessellation shaders. Debug visualization is also used for patch lines.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/TessellationShaders/TerrainHeightmapTessellation.png?raw=true)

## Controls

| Input   | Action                           |
|---------|----------------------------------|
| W/A/S/D | Move the camera                  |
| Mouse   | Look around with the camera      |
| Esc     | Close the window                 |
| 0       | Disable path lines visualization |
| 1       | Enable path lines visualization  |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |
| AppSettings.PolygonMode      | VkPolygonMode     | AppSettings::PolygonMode      | Polygon mode                     |               |

## Learning Objectives

- Creating terrains with using heightmaps
- Understanding and applying quad tessellation in tessellation shaders
- Visualizing lines that passing through patch points in terrain

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
