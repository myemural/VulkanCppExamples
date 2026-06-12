# Drawing Multicolor Triangles

**Code Name:** DrawingMulticolorTriangles

## Description

This example draws 4 different colored triangles to the screen to show how to work with the different vertex attributes.

## Output

![](/Docs/ExampleMedia/Fundamentals/Basics/DrawingMulticolorTriangles.png?raw=true)

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

- Provide multiple vertex attributes to the vertex buffer
- Using multiple vertex attributes in the shader

## Shader Status

| Shader Type                                                            | Status             | Notes |
|------------------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/Basics/DrawingMulticolorTriangles/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/Basics/DrawingMulticolorTriangles/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/Basics/DrawingMulticolorTriangles/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
