# Model Tessellation with Curved PN Triangles

**Code Name:** CurvedPnTrianglesTessellation

## Description

In this example, subdivision is applied to a loaded model using tessellation shaders with the curved PN triangles technique, and the number of triangles in the model has been increased to match the model's curved structure.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/TessellationShaders/CurvedPnTrianglesTessellation.png?raw=true)

## Controls

| Input        | Action                     |
|--------------|----------------------------|
| Mouse Move   | Rotataing the orbit camera |
| Mouse Scroll | Zooming the orbit camera   |
| Esc          | Close the window           |

## Application Parameters

### Settings

| Parameter / Key               | Type              | Usage in Code                  | Description                    | Default Value |
|-------------------------------|-------------------|--------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor        | VkClearColorValue | AppSettings::ClearColor        | Background color of the screen |               |
| AppSettings.MouseSensitivity  | float             | AppSettings::MouseSensitivity  | Mouse sensitivity value        |               |
| AppSettings.CameraZoomSpeed   | float             | AppSettings::CameraZoomSpeed   | Zoom speed of the camera       |               |
| AppSettings.PolygonMode       | VkPolygonMode     | AppSettings::PolygonMode       | Polygon mode                   |               |

## Learning Objectives

TBD

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :x:                | Will be implemented. |
| Slang       | :x:                | Will be implemented. |

## Theoretical Background

- Implementing curved PN triangles technique in Vulkan with using tessellation shaders

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
