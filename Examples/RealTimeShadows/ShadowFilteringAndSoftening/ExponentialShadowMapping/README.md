# Exponential Shadow Mapping (ESM)

**Code Name:** ExponentialShadowMapping

## Description

In this example, the shadows in the scene are created using Exponential Shadow Mapping (ESM) with using box filtering that can be adjusted from user.

## Output

![](/Docs/ExampleMedia/RealTimeShadows/ShadowFilteringAndSoftening/ExponentialShadowMapping.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |
| 0       | Apply no filter             |
| 1       | Apply 3x3 filter            |
| 2       | Apply 5x5 filter            |
| 3       | Apply 7x7 filter            |
| 4       | Apply 9x9 filter            |

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
| AppSettings.EsmExponent      | float             | AppSettings::EsmExponent      | ESM coefficient                  |               |


## Learning Objectives

- Implementing Exponential Shadow Mapping (ESM) technique in Vulkan
- Applying box filtering kernel to Exponential Shadow Mapping (ESM)

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
