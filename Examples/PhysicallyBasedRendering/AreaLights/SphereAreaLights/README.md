# Sphere Area Lights with Representative Point Method

**Code Name:** SphereAreaLights

## Description

This example demonstrates the implementation of sphere area lights with using Representative Point method.

## Output

![](/Docs/ExampleMedia/PhysicallyBasedRendering/AreaLights/SphereAreaLights.png?raw=true)

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
| AppSettings.LightRadius            | float             | AppSettings::LightRadius            | Radius of the light                               |               |
| AppSettings.LightIntensity         | float             | AppSettings::LightIntensity         | Intensity of the area light                       |               |
| AppSettings.FloorRoughness         | float             | AppSettings::FloorRoughness         | Roughness of the floor material                   |               |
| AppSettings.FloorMetallic          | float             | AppSettings::FloorMetallic          | Metallic of the floor material                    |               |

## Learning Objectives

- Implementing sphere area lights using Representative Point method in Vulkan

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
