# Point Light

**Code Name:** PointLight

## Description

In this example, objects of different colors are drawn on the scene, and lighting is provided by a single moving point light.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/LightSources/PointLight.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                                 | Default Value |
|------------------------------|-------------------|-------------------------------|---------------------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen              |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value                     |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera                         |               |
| AppSettings.LightColor       | glm:vec3          | AppSettings::LightColor       | Color of the light (RGB)                    |               |
| AppSettings.AmbientStrength  | float             | AppSettings::AmbientStrength  | Ambient strength of the material            |               |
| AppSettings.SpecularStrength | float             | AppSettings::SpecularStrength | Specular strength factor                    |               |
| AppSettings.Shininess        | float             | AppSettings::Shininess        | Shininess exponent                          |               |
| AppSettings.ConstantFactor   | float             | AppSettings::ConstantFactor   | Constant factor in the attenuation formula  |               |
| AppSettings.LinearFactor     | float             | AppSettings::LinearFactor     | Linear factor in the attenuation formula    |               |
| AppSettings.QuadraticFactor  | float             | AppSettings::QuadraticFactor  | Quadratic factor in the attenuation formula |               |


## Learning Objectives

- Implementing point light behavior in the shaders
- Integrating blinn-phong shading model into the point lights
- Implementing attenuation calculation to the shaders for distance-based lighting

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
