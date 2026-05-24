# Rectangular Area Lights with Linearly Transformed Cosines (LTC)

**Code Name:** RectangularAreaLights

## Description

This example demonstrates the implementation of rectangular area lights with using Linearly Transformed Cosines (LTC) method.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/AreaLights/RectangularAreaLights.png?raw=true)

## Controls

| Input   | Action                      |
|---------|-----------------------------|
| W/A/S/D | Move the camera             |
| Mouse   | Look around with the camera |
| Esc     | Close the window            |

## Application Parameters

### Settings

| Parameter / Key                    | Type              | Usage in Code                       | Description                                       | Default Value |
|------------------------------------|-------------------|-------------------------------------|---------------------------------------------------|---------------|
| AppSettings.ClearColor             | VkClearColorValue | AppSettings::ClearColor             | Background color of the screen                    |               |
| AppSettings.MouseSensitivity       | float             | AppSettings::MouseSensitivity       | Mouse sensitivity value                           |               |
| AppSettings.CameraSpeed            | float             | AppSettings::CameraSpeed            | Speed of the camera                               |               |
| AppSettings.LightColor             | glm:vec3          | AppSettings::LightColor             | Color of the area light                           |               |
| AppSettings.LightIntensity         | float             | AppSettings::LightIntensity         | Intensity of the area light                       |               |
| AppSettings.IsAreaLightDoubleSided | bool              | AppSettings::IsAreaLightDoubleSided | Specifies whether the area light is double-sided. |               |
| AppSettings.FloorRoughness         | float             | AppSettings::FloorRoughness         | Roughness of the floor material                   |               |
| AppSettings.FloorMetallic          | float             | AppSettings::FloorMetallic          | Metallic of the floor material                    |               |

## Learning Objectives

- Implementing rectangular area lights using Linearly Transformed Cosines (LTC) method in Vulkan
- Applying the logic of double-sided area lighting

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
