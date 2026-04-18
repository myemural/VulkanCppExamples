# Mandelbrot Fractal Rendering

**Code Name:** MandelbrotFractal

## Description

In this example, a Mandelbrot fractal is drawn on the screen and the zoom is made towards the region called Seahorse Valley with the speed value taken from the user.

## Output

![](/Docs/ExampleMedia/Fundamentals/ComputeShaders/MandelbrotFractal.png?raw=true)

## Controls

| Input | Action           |
|-------|------------------|
| Esc   | Close the window |

## Application Parameters

### Settings

| Parameter / Key        | Type              | Usage in Code           | Description                    | Default Value |
|------------------------|-------------------|-------------------------|--------------------------------|---------------|
| AppSettings.LoopTime   | float             | AppSettings::LoopTime   | Time interval for camera reset |               |
| AppSettings.ZoomSpeed  | float             | AppSettings::ZoomSpeed  | Zoom speed of the camera       |               |


## Learning Objectives

- Writing more complex compute shaders

## Shader Status

| Shader Type | Status             | Notes |
|-------------|--------------------|-------|
| GLSL        | :white_check_mark: |       |
| HLSL        | :white_check_mark: |       |
| Slang       | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
