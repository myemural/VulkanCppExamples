# Fresnel Effect

**Code Name:** FresnelEffect

## Description

In this example, the Fresnel Effect is implemented on the shader side and applied to a plane surface. When you look at the surface perpendicularly with the camera, the reflection will appear very small, but when you look at it at an angle, the reflection will increase.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/EnvironmentMapping/FresnelEffect.png?raw=true)

## Controls

| Input        | Action                     |
|--------------|----------------------------|
| Mouse Move   | Rotataing the orbit camera |
| Mouse Scroll | Zooming the orbit camera   |
| Esc          | Close the window           |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraZoomSpeed  | float             | AppSettings::CameraZoomSpeed  | Zoom speed of the camera         |               |

## Learning Objectives

- Implementing Fresnel Effect in shaders for better reflections

## Shader Status

| Shader Type | Status             | Notes                |
|-------------|--------------------|----------------------|
| GLSL        | :white_check_mark: |                      |
| HLSL        | :white_check_mark: |                      |
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
