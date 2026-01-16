# Bump Mapping

**Code Name:** BumpMapping

## Description

In this example, bump mapping technique is applied to the objects in the scene. For calculation purposes, a heightmap texture is added to the material information given to the objects.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/SurfaceDetailing/BumpMapping.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                      | Default Value |
|------------------------------|-------------------|-------------------------------|----------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen   |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value          |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera              |               |
| AppSettings.LightDirection   | glm:vec3          | AppSettings::LightDirection   | Direction vector of the light    |               |
| AppSettings.LightColor       | glm:vec3          | AppSettings::LightColor       | Color of the light (RGB)         |               |
| AppSettings.AmbientStrength  | float             | AppSettings::AmbientStrength  | Ambient strength of the material |               |
| AppSettings.SpecularStrength | float             | AppSettings::SpecularStrength | Specular strength factor         |               |
| AppSettings.Shininess        | float             | AppSettings::Shininess        | Shininess exponent               |               |


## Learning Objectives

- Implementing bump mapping technique with using heightmaps
- Using derivative functions in shaders

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
