# Diffuse Lighting in Goraud Shading

**Code Name:** DiffuseLightingGouraud

## Description

In this example, objects of various colors are drawn on the screen, and a rotating light source is added around these objects. Diffuse lighting is applied to these objects in the scene using gouraud shading.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/BasicLighting/DiffuseLightingGouraud.png?raw=true)

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
| AppSettings.LightColor       | glm:vec3          | AppSettings::LightColor       | Color of the light (RGB)       |               |


## Learning Objectives

- Implementing diffuse lighting in gouraud shading model
- Doing light calculations in vertex shaders

## Shader Status

| Shader Type                                                                 | Status             | Notes                |
|-----------------------------------------------------------------------------|--------------------|----------------------|
| [GLSL](/Shaders/RealTimeLighting/BasicLighting/DiffuseLightingGouraud/glsl) | :white_check_mark: |                      |
| [HLSL](/Shaders/RealTimeLighting/BasicLighting/DiffuseLightingGouraud/hlsl) | :white_check_mark: |                      |
| Slang                                                                       | :x:                | Will be implemented. |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
