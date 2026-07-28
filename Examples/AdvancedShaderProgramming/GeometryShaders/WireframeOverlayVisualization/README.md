# Wireframe Overlay Visualization

**Code Name:** WireframeOverlayVisualization

## Description

In this example, wireframe overlay visualization is performed on objects in the scene using the geometry shader and depth bias.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/GeometryShaders/WireframeOverlayVisualization.png?raw=true)

## Controls

| Input   | Action                                  |
|---------|-----------------------------------------|
| W/A/S/D | Move the camera                         |
| Mouse   | Look around with the camera             |
| Esc     | Close the window                        |
| 0       | Disable wireframe overlay visualization |
| 1       | Enable wireframe overlay visualization  |

## Application Parameters

### Settings

| Parameter / Key                | Type              | Usage in Code                   | Description                    | Default Value |
|--------------------------------|-------------------|---------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor         | VkClearColorValue | AppSettings::ClearColor         | Background color of the screen |               |
| AppSettings.MouseSensitivity   | float             | AppSettings::MouseSensitivity   | Mouse sensitivity value        |               |
| AppSettings.CameraSpeed        | float             | AppSettings::CameraSpeed        | Speed of the camera            |               |
| AppSettings.WireframeLineWidth | float             | AppSettings::WireframeLineWidth | Width of the wireframe lines   |               |

## Learning Objectives

- Visualizing wireframe overlays using geometry shaders

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
