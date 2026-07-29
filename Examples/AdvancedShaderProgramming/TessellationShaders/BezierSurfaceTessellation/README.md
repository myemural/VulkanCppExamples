# Bézier Surface with Tessellation Shaders

**Code Name:** BezierSurfaceTessellation

## Description

In this example, a Bézier surface is drawn with 16 control points using tessellation shaders. The orbit camera is used, and the control points can be moved with the mouse.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/TessellationShaders/BezierSurfaceTessellation.png?raw=true)

## Controls

| Input                    | Action                                   |
|--------------------------|------------------------------------------|
| Mouse Left + Mouse Move  | Moving the control points of the surface |
| Mouse Right + Mouse Move | Rotating the orbit camera                |
| Mouse Scroll             | Zooming the orbit camera                 |
| Esc                      | Close the window                         |

## Application Parameters

### Settings

| Parameter / Key               | Type              | Usage in Code                  | Description                    | Default Value |
|-------------------------------|-------------------|--------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor        | VkClearColorValue | AppSettings::ClearColor        | Background color of the screen |               |
| AppSettings.MouseSensitivity  | float             | AppSettings::MouseSensitivity  | Mouse sensitivity value        |               |
| AppSettings.CameraZoomSpeed   | float             | AppSettings::CameraZoomSpeed   | Zoom speed of the camera       |               |
| AppSettings.PolygonMode       | VkPolygonMode     | AppSettings::PolygonMode       | Polygon mode                   |               |
| AppSettings.TessellationLevel | float             | AppSettings::TessellationLevel | Level of tessellation          |               |
| AppSettings.ControlPointSize  | float             | AppSettings::ControlPointSize  | Size of the control points     |               |
| AppSettings.ControlPointColor | glm::vec3         | AppSettings::ControlPointColor | Color of the control points    |               |

## Learning Objectives

TBD

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :x:                | Will be implemented. |
| Slang       | :x:                | Will be implemented. |

## Theoretical Background

- Learning how to implement Bézier surfaces with tessellation shaders
- Learning how to manipulate control points for surface deformation
- Understanding how patch topology is working on more than 4 patch vertices

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
