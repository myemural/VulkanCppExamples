# Normal Vector Visualization

**Code Name:** NormalVectorVisualization

## Description

In this example, the geometry shader is used to visualize the surface normal vectors of the meshes.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/GeometryShaders/NormalVectorVisualization.png?raw=true)

## Controls

| Input   | Action                              |
|---------|-------------------------------------|
| W/A/S/D | Move the camera                     |
| Mouse   | Look around with the camera         |
| Esc     | Close the window                    |
| 0       | Disable normal vector visualization |
| 1       | Enable normal vector visualization  |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |

## Learning Objectives

- Visualizing surface normal vectors with using geometry shaders

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
