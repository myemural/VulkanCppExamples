# LOD Rendering with Task Shader

**Code Name:** LodTaskShader

## Description

This example demonstrates how to apply LOD using a task shader, based on the distance of the models with meshlets drawn on the scene from the camera.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/MeshAndTaskShaders/LodTaskShader.png?raw=true)

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

## Learning Objectives

- Showing the LOD of the model based on camera distance using task shaders

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
- VK_KHR_spirv_1_4
- VK_EXT_mesh_shader
- VK_KHR_shader_float_controls
