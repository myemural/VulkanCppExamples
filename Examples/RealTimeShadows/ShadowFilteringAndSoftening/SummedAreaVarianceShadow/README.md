# Summed-Area Variance Shadow Mapping (SAVSM)

**Code Name:** SummedAreaVarianceShadow

## Description

In this example, to reduce the filtering cost to O(1) complexity, Summed Area Table (SAT) is applied to the shadow map and the shadows in the scene are demonstrated using the Summed Area Variance Shadow Mapping (SAVSM) technique.

## Output

![](/Docs/ExampleMedia/RealTimeShadows/ShadowFilteringAndSoftening/SummedAreaVarianceShadow.png?raw=true)

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


## Learning Objectives

- Implementing Summed Area Variance Shadow Mapping (SAVSM) technique in Vulkan
- Generated Summed Area Table with compute shaders
- Applying box filtering kernel to Summed Area Variance Shadow Mapping (SAVSM)

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
