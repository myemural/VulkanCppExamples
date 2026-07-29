# Cubic Bézier Curve with Tessellation Shaders

**Code Name:** CubicBezierCurveTessellation

## Description

In this example, a cubic Bézier curve is drawn using tessellation shaders with 4 control points. The control points can be moved with the mouse.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/TessellationShaders/CubicBezierCurveTessellation.png?raw=true)

## Controls

| Input                   | Action                                 |
|-------------------------|----------------------------------------|
| Mouse Left + Mouse Move | Moving the control points of the curve |
| Esc                     | Close the window                       |

## Application Parameters

### Settings

| Parameter / Key               | Type              | Usage in Code                  | Description                    | Default Value |
|-------------------------------|-------------------|--------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor        | VkClearColorValue | AppSettings::ClearColor        | Background color of the screen |               |
| AppSettings.TessellationLevel | float             | AppSettings::TessellationLevel | Level of tessellation          |               |
| AppSettings.ControlPointSize  | float             | AppSettings::ControlPointSize  | Size of the control points     |               |
| AppSettings.ControlPointColor | glm::vec3         | AppSettings::ControlPointColor | Color of the control points    |               |
| AppSettings.CurveColor        | glm::vec3         | AppSettings::CurveColor        | Color of the curve             |               |

## Learning Objectives

TBD

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :x:                | Will be implemented. |
| Slang       | :x:                | Will be implemented. |

## Theoretical Background

- Understanding how to implement cubic Bézier curves with using isolines tessellation
- Understanding how isoline tessellation is working
- Learning how to drag and drop any points with mouse in Vulkan applications

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
