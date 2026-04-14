# Omnidirectional Shadow Mapping with Point Lights

**Code Name:** OmnidirectionalShadowMapping

## Description

This example demonstrates an omnidirectional shadow created by a point light rotating in a circle in the center of a closed room.

## Output

![](/Docs/ExampleMedia/RealTimeShadows/BasicShadowMapping/OmnidirectionalShadowMapping.jpg?raw=true)

## Controls

| Input   | Action                               |
|---------|--------------------------------------|
| W/A/S/D | Move the camera                      |
| Mouse   | Look around with the camera          |
| Esc     | Close the window                     |

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

- Implementing omnidirectional shadow mapping technique for point light sources in Vulkan
- Using cubemaps for generating shadow maps

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
