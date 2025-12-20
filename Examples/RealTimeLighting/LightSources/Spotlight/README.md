# Spotlight

**Code Name:** Spotlight

## Description

In this example, objects of different colors are drawn on the scene, and lighting is provided by two spotlights with hard-cutoff.

## Output

![](/Docs/ExampleMedia/RealTimeLighting/LightSources/Spotlight.png?raw=true)

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
| AppSettings.CutoffAngle      | float             | AppSettings::CutoffAngle      | Cutoff angle of the spotlight    |               |


## Learning Objectives

- Implementing spotlight behavior in the shaders
- Integrating blinn-phong shading model into the spotlights
- Implementing hard-cutoff (hard edges) spotlight behavior with only one cutoff angle
- Implementing multiple lights behavior with the same type

## Theoretical Background

None

## Extensions Used

### Instance

Window system-dependent extensions:
- VK_KHR_surface
- VK_KHR_win32_surface (Windows)

### Device

- VK_KHR_swapchain
