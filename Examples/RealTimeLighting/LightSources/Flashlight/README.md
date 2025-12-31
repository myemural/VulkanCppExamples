# Flashlight with Soft-Cutoff

**Code Name:** Flashlight

## Description

In this example, objects of different colors are drawn on the scene, and lighting is provided by a flashlight with soft-cutoff that coming from the camera.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/LightSources/Flashlight.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key              | Type              | Usage in Code                 | Description                         | Default Value |
|------------------------------|-------------------|-------------------------------|-------------------------------------|---------------|
| AppSettings.ClearColor       | VkClearColorValue | AppSettings::ClearColor       | Background color of the screen      |               |
| AppSettings.MouseSensitivity | float             | AppSettings::MouseSensitivity | Mouse sensitivity value             |               |
| AppSettings.CameraSpeed      | float             | AppSettings::CameraSpeed      | Speed of the camera                 |               |
| AppSettings.LightDirection   | glm:vec3          | AppSettings::LightDirection   | Direction vector of the light       |               |
| AppSettings.LightColor       | glm:vec3          | AppSettings::LightColor       | Color of the light (RGB)            |               |
| AppSettings.AmbientStrength  | float             | AppSettings::AmbientStrength  | Ambient strength of the material    |               |
| AppSettings.SpecularStrength | float             | AppSettings::SpecularStrength | Specular strength factor            |               |
| AppSettings.Shininess        | float             | AppSettings::Shininess        | Shininess exponent                  |               |
| AppSettings.InnerCutoffAngle | float             | AppSettings::InnerCutoffAngle | Inner cutoff angle of the spotlight |               |
| AppSettings.OuterCutoffAngle | float             | AppSettings::OuterCutoffAngle | Outer cutoff angle of the spotlight |               |


## Learning Objectives

- Implementing camera-based spotlight (flashlight) behavior
- Implementing soft-cutoff (soft edges) spotlight behavior with two cutoff angles

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
