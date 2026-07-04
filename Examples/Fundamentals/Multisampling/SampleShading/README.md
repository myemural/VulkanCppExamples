# Sample Shading

**Code Name:** SampleShading

## Description

In this example, cubes were drawn on the screen using the sample shader. The first sample was colored red, and the others were textured. Furthermore, using the sample shader eliminated the texture aliasing problem.

## Output

![](/Docs/ExampleMedia/Fundamentals/Multisampling/SampleShading.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                    | Default Value |
|------------------------------|-------------------|-------------------------------|--------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value        |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera            |               |


## Learning Objectives

- Enabling sample shading
- Writing fragment shaders based on samples

## Shader Status

| Shader Type                                                      | Status             | Notes |
|------------------------------------------------------------------|--------------------|-------|
| [GLSL](/Shaders/Fundamentals/Multisampling/SampleShading/glsl)   | :white_check_mark: |       |
| [HLSL](/Shaders/Fundamentals/Multisampling/SampleShading/hlsl)   | :white_check_mark: |       |
| [Slang](/Shaders/Fundamentals/Multisampling/SampleShading/slang) | :white_check_mark: |       |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
