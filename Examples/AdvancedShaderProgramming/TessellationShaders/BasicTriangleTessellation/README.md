# Basic Triangle Tessellation

**Code Name:** BasicTriangleTessellation

## Description

In this example, a simple triangle is drawn on the screen in wireframe mode, and tessellation shaders are applied to this triangle, dividing its interior into smaller triangles. The inner tessellation level value is set by the user.

## Output

![](/Docs/ExampleMedia/AdvancedShaderProgramming/TessellationShaders/BasicTriangleTessellation.png?raw=true)

## Controls

| Input   | Action                          |
|---------|---------------------------------|
| Esc     | Close the window                |
| 0       | Inner Tessellation Level = 0.0  |
| 1       | Inner Tessellation Level = 3.0  |
| 2       | Inner Tessellation Level = 5.0  |
| 3       | Inner Tessellation Level = 8.0  |
| 4       | Inner Tessellation Level = 10.0 |
| 5       | Inner Tessellation Level = 12.0 |
| 6       | Inner Tessellation Level = 15.0 |
| 7       | Inner Tessellation Level = 17.0 |
| 8       | Inner Tessellation Level = 20.0 |

## Application Parameters

### Settings

| Parameter / Key        | Type              | Usage in Code           | Description                    | Default Value |
|------------------------|-------------------|-------------------------|--------------------------------|---------------|
| AppSettings.ClearColor | VkClearColorValue | AppSettings::ClearColor | Background color of the screen |               |
| AppSettings.LineWidth  | float             | AppSettings::LineWidth  | Width of the wireframe lines   |               |

## Learning Objectives

- Learning how to use tessellation shaders in Vulkan
- Understanding how tessellation control variables are working
- Implementing basic tessellation evaluation shader
- Understanding how triangle tessellation is working

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
