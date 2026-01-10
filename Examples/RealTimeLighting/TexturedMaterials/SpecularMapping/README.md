# Specular Mapping

**Code Name:** SpecularMapping

## Description

In this example, specular map texture is used instead of specular color in the lighting calculations for textured objects drawn on the screen.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/TexturedMaterials/SpecularMapping.png?raw=true)

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

- Applying specular texture to objects via shaders
- Including specular texture to the lighting calculations (for blinn-phong)

## Shader Status

| Shader Type | Status             | Notes                                                                     |
|-------------|--------------------|---------------------------------------------------------------------------|
| GLSL        | :white_check_mark: |                                                                           |
| HLSL        | :white_check_mark: | Get array length from constants is not working, using workaround for now. |
| Slang       | :x:                | Will be implemented.                                                      |

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
