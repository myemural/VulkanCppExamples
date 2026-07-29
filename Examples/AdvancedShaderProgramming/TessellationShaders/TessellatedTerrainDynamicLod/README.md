# Tessellated Terrain Rendering with Dynamic LOD

**Code Name:** TessellatedTerrainDynamicLod

## Description

This example demonstrates how to create a tessellated terrain using heightmap and tessellation shaders, and how to apply dynamic LOD to the tessellated regions of this terrain based on camera distance. Also T-junction/Crack problem has been prevented.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/TessellationShaders/TessellatedTerrainDynamicLod.png?raw=true)

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
| AppSettings.MinTessDistance  | float             | AppSettings::MinTessDistance  | Minimum tessellation distance    |               |
| AppSettings.MaxTessDistance  | float             | AppSettings::MaxTessDistance  | Maximum tessellation distance    |               |
| AppSettings.MinTessLevel     | float             | AppSettings::MinTessLevel     | Minimum tessellation level       |               |
| AppSettings.MaxTessLevel     | float             | AppSettings::MaxTessLevel     | Maximum tessellation level       |               |

## Learning Objectives

- Implementing dynamic LOD on tessellated terrains in Vulkan
- Change tessellation amount per patch according to camera distance in tessellation control shader

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
