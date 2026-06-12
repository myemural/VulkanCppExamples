# Drawing Multiple Triangles

**Code Name:** DrawingMultipleTriangles

## Description

This example draws 4 identical triangles to the screen to show how to draw something to the screen with using vertex buffer.

## Output

![](/Docs/ExampleMedia/Fundamentals/Basics/DrawingMultipleTriangles.png?raw=true)

## Controls

| Input | Action           |
|-------|------------------|
| Esc   | Close the window |

## Application Parameters

### Settings

| Parameter / Key        | Type              | Usage in Code           | Description                                            | Default Value |
|------------------------|-------------------|-------------------------|--------------------------------------------------------|---------------|
| AppSettings.ClearColor | VkClearColorValue | AppSettings::ClearColor | Specifies which color the screen will be cleared with. |               |


## Learning Objectives

- Creating a vertex buffer and fill it with data
- Binding vertex buffer with command
- Using vertex buffer in a shader with vertex attributes

## Shader Status

| Shader Type                                                          | Status             | Notes |
|----------------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/Basics/DrawingMultipleTriangles/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/Basics/DrawingMultipleTriangles/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/Basics/DrawingMultipleTriangles/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
